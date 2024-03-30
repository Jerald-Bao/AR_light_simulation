/*
    Jerald Bao
    Spring 2024 CS 5330
    3/10/2024
    
    calibration.cpp
    includes different functionalities of the tasks of this project
*/

#include "opencv2/opencv.hpp"
#include <util.h>
#include <render.h>

std::vector<cv::Point2f> corner_set, cached_corner_set;	
cv::Mat cached_capture;
std::vector<cv::Vec3f> point_set;
std::vector<std::vector<cv::Vec3f>> point_list;
std::vector<std::vector<cv::Point2f>> corner_list;
std::vector<cv::Mat> captures;
cv::Size patternSize(9, 6);// Number of inner corners per chessboard row and column
std::vector<cv::Vec3f> reverse_point_set;

/*
initialize the point set.
reverse_point_set is for opengl due to the different coord system they use.
*/
void init_point_set(){
    if (point_set.size() == 0) {
        for (int j=0; j<6; j++) {
            for (int i=0; i<9; i++)
                point_set.push_back(cv::Vec3f(i,-j,0));
            }
    }
    if (reverse_point_set.size() == 0) {
        for (int j=0; j<6; j++) {
            for (int i=0; i<9; i++)
                reverse_point_set.push_back(cv::Vec3f(i,j,0));
            }
    }
}


/*
detect the corners of a chessboard.
*/
bool detectCorner(cv::Mat& img, std::vector<cv::Point2f>& corner_set, cv::Mat& dst, bool save_points) {
    init_point_set();
    img.copyTo(dst);
     
    bool found = findChessboardCorners(img, patternSize, corner_set, 
    cv::CALIB_CB_ADAPTIVE_THRESH + cv::CALIB_CB_NORMALIZE_IMAGE +cv::CALIB_CB_FAST_CHECK);
    if (save_points) {
        if (found) {
            corner_list.push_back(std::vector<cv::Point2f>(corner_set));
            point_list.push_back(std::vector<cv::Vec3f>(point_set));
            captures.push_back(img);
        } else {
            corner_list.push_back(std::vector<cv::Point2f>(cached_corner_set));
            point_list.push_back(std::vector<cv::Vec3f>(point_set));
            captures.push_back(cached_capture);
        }
    } else {
        cached_capture = img;
        cached_corner_set = corner_set;
    }
    if (found) {
        cv::drawChessboardCorners(dst, patternSize, cv::Mat(corner_set), found);
        std::cout << "Detected Corners" << std::endl;
    } else {
        std::cout << "Corners not found" << std::endl;
    }
    return found;
}

/*
read camera matrix and distortion coefficients from the yaml file
*/
void read_camera(cv::Mat& cameraMatrix, cv::Mat& distCoeffs) {
    std::string filename = "camera_parameters.yml";
    cv::FileStorage fileStorage(filename, cv::FileStorage::READ);
    if (!fileStorage.isOpened()) {
        std::cerr << "Failed to open " << filename << " for reading." << std::endl;
        return;
    }

    // Variables to store the camera matrix and distortion coefficients

    // Read the camera matrix and distortion coefficients from the file
    fileStorage["camera_matrix"] >> cameraMatrix;
    fileStorage["distortion_coefficients"] >> distCoeffs;

    // Close the file storage
    fileStorage.release();
}

/*
write camera matrix and distortion coefficients to the yaml file
*/
void write_camera(cv::Mat& cameraMatrix, cv::Mat& distCoeffs) {
    std::string filename = "camera_parameters.yml";
    cv::FileStorage fs(filename, cv::FileStorage::WRITE);
    if (!fs.isOpened())
    {
        std::cerr << "Failed to open calibration_parameters.yaml" << std::endl;
        return;
    }

    // Write the camera matrix and distortion coefficients
    fs << "camera_matrix" << cameraMatrix;
    fs << "distortion_coefficients" << distCoeffs;

    // Close the file
    fs.release();

    std::cout << "Calibration parameters saved successfully." << std::endl;
}

/*
read the initial value of camera matrix and distortion coefficients from the yaml file
*/
void read_camera_initial(cv::Mat& cameraMatrix, cv::Mat& distCoeffs) {
    std::string filename = "camera_parameters_initial.yml";
    cv::FileStorage fileStorage(filename, cv::FileStorage::READ);
    if (!fileStorage.isOpened()) {
        std::cerr << "Failed to open " << filename << " for reading." << std::endl;
        return;
    }

    // Variables to store the camera matrix and distortion coefficients

    // Read the camera matrix and distortion coefficients from the file
    fileStorage["camera_matrix"] >> cameraMatrix;
    fileStorage["distortion_coefficients"] >> distCoeffs;

    // Close the file storage
    fileStorage.release();
}

cv::Mat cameraMatrix, distCoeffs;

/*
use the saved images and data to calibrate the camera.
*/
void calibrate(cv::Mat& img) {
    if (corner_list.size() > 5) {
        if (cameraMatrix.rows < 1)
            read_camera_initial(cameraMatrix, distCoeffs);


        std::vector<cv::Mat> rvecs, tvecs; // Output rotation and translation vectors
        
        std::cout << "before calibration:"<<std::endl;
        std::cout << "focal length:" << cameraMatrix.at<double>(0,0) << ", u0: " << cameraMatrix.at<double>(0,2)<<" , v0: "<< cameraMatrix.at<double>(1,2)<< std::endl;
        std::cout << "dist Coeffs: " << distCoeffs.at<double>(0,0) << ", " << distCoeffs.at<double>(1,0)<<" ,"<< distCoeffs.at<double>(2,0)<< std::endl;
        
        // Perform calibration
        double rms = cv::calibrateCamera(point_list, corner_list, cv::Size(640, 480), cameraMatrix, distCoeffs, rvecs, tvecs);

        std::vector<cv::Point2f> projectedPoints;
        double totalError=0;
        double totalPoints=0;
        std::cout << "after calibration:"<<std::endl;
        std::cout << "focal length:" << cameraMatrix.at<double>(0,0) << ", u0: " << cameraMatrix.at<double>(0,2)<<" , v0: "<< cameraMatrix.at<double>(1,2)<< std::endl;
        std::cout << "dist Coeffs: " << distCoeffs.at<double>(0,0) << ", " << distCoeffs.at<double>(1,0)<<" ,"<< distCoeffs.at<double>(2,0)<< std::endl;
        for (int i=0;i< rvecs.size(); i++) {
            std::cout << "rvecs[" << i << "]: " << rvecs[i].at<double>(0,0)<<" "<< rvecs[i].at<double>(1,0)<<" "<< rvecs[i].at<double>(2,0)<<" " << std::endl;
            std::cout << "tvecs[" << i << "]: " << tvecs[i].at<double>(0,0)<<" "<< tvecs[i].at<double>(1,0)<<" "<< tvecs[i].at<double>(2,0)<<" " << std::endl;
            cv::projectPoints(point_list[i], rvecs[i], tvecs[i], cameraMatrix, distCoeffs, projectedPoints);

            // Calculate the error between the projected points and the original image points
            double error = cv::norm(corner_list[i], projectedPoints, cv::NORM_L2);

            // Accumulate the error
            totalError += error * error; // Squared L2 norm
            totalPoints += point_list[i].size();
            std::cout<< "error: "<< error <<"\n";
            if (i == rvecs.size() - 1) {
                cv::Mat demo;
                captures[i].copyTo(demo);
                cv::drawChessboardCorners(demo, patternSize, cv::Mat(projectedPoints), true);
                cv::imshow("demo", demo);
            }
        }
        write_camera(cameraMatrix,distCoeffs);
    } else {
        std::cout<< "not enough calibration images"<<"\n";
    }
}

/*
get the target's pose using solvePnP
*/
bool getBoardTransformation(cv::Mat& img,  std::vector<cv::Point2f>& corner_set, cv::Mat& dst,cv::Mat& rvec, cv::Mat& tvec, bool showBoard) { 
    init_point_set();
    cv::Mat cameraMatrix, distCoeffs;
    read_camera(cameraMatrix, distCoeffs);

    // Solve for pose
    cv::solvePnP( point_set,corner_set, cameraMatrix, distCoeffs, rvec, tvec);
    std::cout << "rvecs: " << rvec.at<double>(0,0)<<" "<< rvec.at<double>(1,0)<<" "<< rvec.at<double>(2,0)<<" " << std::endl;
    std::cout << "tvecs: " << tvec.at<double>(0,0)<<" "<< tvec.at<double>(1,0)<<" "<< tvec.at<double>(2,0)<<" " << std::endl;
    if (showBoard) {
        std::vector<cv::Point2f> projectedPoints;
        cv::projectPoints(point_set, rvec, tvec, cameraMatrix, distCoeffs, projectedPoints);

        // Calculate the error between the projected points and the original image points
        double error = cv::norm(corner_set, projectedPoints, cv::NORM_L2);

        std::cout<< "error: "<< error <<"\n";
        cv::Mat demo;
        img.copyTo(demo);
        cv::drawChessboardCorners(demo, patternSize, cv::Mat(projectedPoints), true);
        cv::imshow("demo", demo);
    }
    return true;
}

/*
draw the meshes in opencv
*/
void render(cv::Mat& img, cv::Mat& dst, cv::Mat& rvec, cv::Mat& tvec, char* filename) { 
    img.copyTo(dst);
    std::vector<cv::Vec3f> vertices, normals;
    std::vector<cv::Vec3i> faces;
    cv::Mat cameraMatrix, distCoeffs;
    loadObj(filename, vertices,normals, faces);
    read_camera(cameraMatrix, distCoeffs);
        std::vector<cv::Point2f> projectedPoints;
    cv::projectPoints(vertices, rvec, tvec, cameraMatrix, distCoeffs, projectedPoints);
    for (const auto &face : faces) {
        drawTriangle(dst, projectedPoints[face[0]], projectedPoints[face[1]], projectedPoints[face[2]]);
    }
}


/*
draw the axes of the target's coordination system.
*/
void axis(cv::Mat& img, cv::Mat& dst, cv::Mat& rvec, cv::Mat& tvec) { 
    img.copyTo(dst);
    std::vector<cv::Vec3f> point_set;
    point_set.push_back(cv::Vec3f(0,0,0));
    point_set.push_back(cv::Vec3f(1,0,0));
    point_set.push_back(cv::Vec3f(0,1,0));
    point_set.push_back(cv::Vec3f(0,0,1));
    if (cameraMatrix.rows<1)
        read_camera(cameraMatrix, distCoeffs);
    std::vector<cv::Point2f> projectedPoints;
    cv::projectPoints(point_set, rvec, tvec, cameraMatrix, distCoeffs, projectedPoints);
    cv::arrowedLine(dst, projectedPoints[0],projectedPoints[1],cv::Scalar(10,10,230),3);
    cv::arrowedLine(dst, projectedPoints[0],projectedPoints[2],cv::Scalar(10,230,10),3);
    cv::arrowedLine(dst, projectedPoints[0],projectedPoints[3],cv::Scalar(230,10,10),3);
    
}

/*
draw the shaded object in opengl
*/
void renderGL(cv::Mat& img, cv::Mat& dst,  std::vector<cv::Point2f>& corner_set, cv::Mat& rvec, cv::Mat& tvec, 
std::vector<cv::Vec3f> v, std::vector<cv::Vec3i> indices,std::vector<cv::Vec3f> normals, GLFWwindow* window,GLint program) { 
    init_point_set();
    cv::Mat cameraMatrix, distCoeffs;
    read_camera(cameraMatrix, distCoeffs);
    // Solve for pose
    cv::solvePnP( reverse_point_set,corner_set, cameraMatrix, distCoeffs, rvec, tvec);
    //rvec.at<double>(2,0) = -rvec.at<double>(2,0);
    std::cout << "rvecs: " << rvec.at<double>(0,0)<<" "<< rvec.at<double>(1,0)<<" "<< rvec.at<double>(2,0)<<" " << std::endl;
    std::cout << "tvecs: " << tvec.at<double>(0,0)<<" "<< tvec.at<double>(1,0)<<" "<< tvec.at<double>(2,0)<<" " << std::endl;
    renderLoop(img,dst, v,indices,normals,rvec,tvec,cameraMatrix,window,program);
}

/*
detect corners using harris corner
*/
void harris_corner(cv::Mat& src,cv::Mat& dst){
    cv::Mat gray;
    src.copyTo(dst);
    cvtColor(src, gray, cv::COLOR_BGR2GRAY);
    cv::Mat harris,dst_norm, dst_norm_scaled;

    // Detector parameters
    int blockSize = 2; // Size of the neighborhood considered for corner detection
    int apertureSize = 3; // Aperture parameter for the Sobel operator
    double k = 0.04; // Harris detector free parameter

    // Detecting corners
    cv::cornerHarris(gray, harris, blockSize, apertureSize, k);

    // Normalizing
    cv::normalize( harris, dst_norm, 0, 255, cv::NORM_MINMAX, CV_32FC1, cv::Mat());
    cv::convertScaleAbs(dst_norm, harris);

    for (int i = 0; i < dst_norm.rows; i++)
    {
        for (int j = 0; j < dst_norm.cols; j++)
        {
            if ((int)dst_norm.at<float>(i, j) > 180) // Threshold for corner detection
            {
                cv::circle(dst, cv::Point(j, i), 5, cv::Scalar(0), 2, 8, 0);
            }
        }
    }
}

