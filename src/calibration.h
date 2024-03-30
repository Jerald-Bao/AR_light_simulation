/*
    Jerald Bao
    Spring 2024 CS 5330
    3/10/2024
    
    calibration.cpp
    includes different functionalities of the tasks of this project
*/


#ifndef CORNER_DETECTION_H
#define CORNER_DETECTION_H
#include "glfw3.h"
/*
detect the corners of a chessboard.
*/
bool detectCorner(cv::Mat& img, std::vector<cv::Point2f>& corner_set, cv::Mat& dst, bool save_points);


/*
use the saved images and data to calibrate the camera.
*/
void calibrate(cv::Mat& img);

/*
get the target's pose using solvePnP
*/
bool getBoardTransformation(cv::Mat& img,  std::vector<cv::Point2f>& corner_set, cv::Mat& dst,cv::Mat& rvec, cv::Mat& tvec, bool showBoard);

/*
draw the meshes in opencv
*/
void render(cv::Mat& img, cv::Mat& dst, cv::Mat& rvec, cv::Mat& tvec, char* filename);

/*
read camera matrix and distortion coefficients from the yaml file
*/
void read_camera(cv::Mat& cameraMatrix, cv::Mat& distCoeffs);

/*
draw the shaded object in opengl
*/
void renderGL(cv::Mat& img, cv::Mat& dst,std::vector<cv::Point2f>& corner_set, cv::Mat& rvec, cv::Mat& tvec, 
std::vector<cv::Vec3f> v, std::vector<cv::Vec3i> indices,std::vector<cv::Vec3f> normals, GLFWwindow* window,GLint program);

/*
detect corners using harris corner
*/
void harris_corner(cv::Mat& src,cv::Mat& dst);


/*
draw the axes of the target's coordination system.
*/
void axis(cv::Mat& img, cv::Mat& dst, cv::Mat& rvec, cv::Mat& tvec);
#endif