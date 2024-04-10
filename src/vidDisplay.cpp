/*
    Jerald Bao
    Spring 2024 CS 5330
    3/10/2024
    
    vidDisplay.cpp
    display a video from webcam and execute different functionalities activated by different keystrokes.
*/
#include "opencv2/opencv.hpp"
#include "vidDisplay.h"
#include "calibration.h"
#include <chrono>
#include <iomanip>
#include <sstream>
#include <ctime>
#include <cstdlib>
#include <cstdio>
#include <util.h>
#include "render.h"
#include "labelTool.h"


/*
    vidDisplay::main
    Display a video from webcam and execute different functionalities activated by different keystrokes. 
    
*/
int main(int argc, char *argv[]) {
    cv::VideoCapture *capdev;
    // open the video device
    capdev = new cv::VideoCapture(1);
    if( !capdev->isOpened() ) {
        printf("Unable to open video device\n");
        return(-1);
    }

    /*
        exclusively used for testing opengl parts
    */
    if (argc >= 2 && strcmp(argv[1], "--test-rendering")==0)
    {
        GLFWwindow *window = init_window();
        GLuint shaderProgram = get_shader_program();
        //linkShader();
        cv::Mat img,dst;
        cv::Mat rvec = cv::Mat(3, 1, CV_64F);
        rvec.at<double>(0,0)=0;
        rvec.at<double>(1,0)=0;
        rvec.at<double>(2,0)=0.;
        cv::Mat tvec = cv::Mat(3, 1, CV_64F);
        tvec.at<double>(0,0)=0;
        tvec.at<double>(1,0)=0;
        tvec.at<double>(2,0)=-5;
        std::vector<cv::Vec3f> v,normal;
        
        std::vector<cv::Vec3i> i;
        loadObj("res/bunny.obj", v,normal, i);
        cv::Mat cameraMatrix, distCoeffs;
        read_camera(cameraMatrix, distCoeffs);
        renderLoop(img,dst,v,i,normal,rvec,tvec,cameraMatrix, window,shaderProgram);
        return 0;
    }

    if (argc >= 2 && strcmp(argv[1], "--label-lighting")==0) {
        lt::showImages("image_label_data.csv","./Data/images/");
        return 0;
    }

    std::vector<cv::Vec3f> v,normal;
    std::vector<cv::Vec3i> indices;
    cv::Mat frame,cached_frame,dst;
    std::vector<cv::Point2f> corner_set, cached_corner_set;	
    std::vector<cv::Vec3f> point_set;
	std::vector<std::vector<cv::Vec3f> > point_list;
	std::vector<std::vector<cv::Point2f> > corner_list;
    GLFWwindow *window; 
    GLint shaderProgram;
    int mode = Mode::origin;

    double fpsTarget = 30;
    double frameInterval = 1000.0 / fpsTarget;  // Interval in milliseconds
    double shearControlX=0;
    double shearControlY=0;
    mode = Mode::origin;
    for(;;) {
        *capdev >> frame; // get a new frame from the camera, treat as a stream
        cv::resize(frame, frame, cv::Size(640,480));
        if( frame.empty() ) {
            printf("frame is empty\n");
            break;
        } 
        cv::Mat corners;
        std::chrono::steady_clock::time_point start = std::chrono::high_resolution_clock::now();   
        if (mode & Mode::origin)
        {
            cv::imshow("Origin", frame);
        }
        if (mode & Mode::corners) {
            bool save = mode & Mode::save;
            bool found = detectCorner(frame, corner_set, corners, save);
            cv::imshow("corners", corners);
            if (save) {
                mode ^= Mode::save;
            }
        } 
        if (mode & Mode::calibration) {
            calibrate(frame);
            mode ^= Mode::calibration;
        }

        // AR related functions
        if (mode & Mode::board) {
            bool found = detectCorner(frame, corner_set, corners, false);
            cv::Mat rvec,tvec;
            if (found) {
                // render a meshed bunney
                if (mode & Mode::render) {
                    getBoardTransformation(frame, corner_set, dst, rvec, tvec,mode & Mode::tranfromation);
                    render(frame, dst, rvec, tvec, "res/bunny.obj");
                    cv::imshow("render", dst);
                } else
                // render a opengl bunney with lighting
                if (mode & Mode::renderGL) {
                    renderGL(frame, dst,corner_set, rvec, tvec, v,indices,normal,window,shaderProgram);
                } else
                // show the target's coordination system
                if (mode & Mode::project)
                {
                    getBoardTransformation(frame, corner_set, dst, rvec, tvec,mode & Mode::tranfromation);
                    axis(frame, dst, rvec, tvec);
                    cv::imshow("axis", dst);
                }
                else {
                    getBoardTransformation(frame, corner_set, dst, rvec, tvec,mode & Mode::tranfromation);
                } 
            }
        }
        if (mode & Mode::harris) {
            harris_corner(frame,dst);
            cv::imshow("harris", dst);
        }


        int renderTime = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::high_resolution_clock::now() - start
        ).count();

        int actualFPS = renderTime < frameInterval ? fpsTarget : 1000 / renderTime;

        cv::putText(frame, "FPS: " + std::to_string(actualFPS), cv::Point(10, 30),
                    cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(10, 10, 10), 2);

        cv::putText(frame, "rendering time: " + std::to_string(renderTime) + " ms", cv::Point(10, 100),
                    cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(10, 10, 10), 2);


        int waitTime = std::max(1, static_cast<int>(frameInterval - renderTime));
        std::this_thread::sleep_for(std::chrono::milliseconds(waitTime));


        // see if there is a waiting keystroke
        char key = cv::waitKey(1);
        if ( key == 'q') {
            break;
        } if (key == 'c') {
            mode ^= Mode::corners;
            cv::destroyAllWindows();
        } if (key == 's') {
            mode ^= Mode::save;
        } if (key == 'l') {
            mode ^= Mode::calibration;
            cv::destroyAllWindows();
        } if (key == 't') {
            mode ^= Mode::tranfromation;
            cv::destroyAllWindows();
        } if (key == 'r') {
            mode ^= Mode::render;
            cv::destroyAllWindows();
        } if (key == 'b') {
            mode ^= Mode::board;
            cv::destroyAllWindows();
        }  if (key == 'g') {
            mode ^= Mode::renderGL;
            cv::destroyAllWindows();
        } if (key == 'i') {
            window = init_window();
            shaderProgram = get_shader_program();
            loadObj("res/bunny.obj", v,normal, indices);
        } if (key == 'h') {
            mode ^= Mode::harris;
            cv::destroyAllWindows();
        } if (key == 'p') {
            mode ^= Mode::project;
            cv::destroyAllWindows();
        }
    }

    delete capdev;
    return(0);
}




