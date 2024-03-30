/*
    Jerald Bao
    Spring 2024 CS 5330
    3/10/2024
    
    util.cpp
    read and parse resource files.
*/

#ifndef UTIL_H
#define UTIL_H
#include <opencv2/opencv.hpp>
#include <fstream>
#include <sstream>
#include <vector>
// Function to load vertices and faces from an OBJ file
void loadObj(const std::string &filename, std::vector<cv::Vec3f> &vertices,std::vector<cv::Vec3f> &normals, std::vector<cv::Vec3i> &faces);
// Draw triangles in opencv
void drawTriangle(cv::Mat &image, const cv::Point2f &v1, const cv::Point2f &v2, const cv::Point2f &v3);
#endif