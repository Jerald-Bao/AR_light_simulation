/*
    Jerald Bao
    Spring 2024 CS 5330
    3/10/2024
    
    util.cpp
    read and parse resource files.
*/

#include <opencv2/opencv.hpp>
#include <fstream>
#include <sstream>
#include <vector>
#include <util.h>
// Function to load vertices and faces from an OBJ file
void loadObj(const std::string &filename, std::vector<cv::Vec3f> &vertices,std::vector<cv::Vec3f> &normals, std::vector<cv::Vec3i> &faces) {
    std::ifstream objFile(filename);
    std::string line;

    while (std::getline(objFile, line)) {
        std::stringstream ss(line);
        std::string type;
        ss >> type;

        if (type == "v") {
            cv::Point3f v;
            ss >> v.x >> v.y >> v.z;
            vertices.push_back(v);
        }if (type == "vn") {
            cv::Point3f normal;
            ss >> normal.x >> normal.y >> normal.z;
            normals.push_back(normal);
        } else if (type == "f") {
            cv::Vec3i face;
            std::string s1,s2,s3;
            ss >> s1 >> s2 >> s3;
            int i1 = s1.find_first_of('/');
            int i2 = s2.find_first_of('/');
            int i3 = s3.find_first_of('/');
            face[0] =  std::stoi(s1.substr(0,i1)); 
            face[1] =  std::stoi(s2.substr(0,i2)); 
            face[2] =  std::stoi(s3.substr(0,i3)); 
            face[0]--;face[1]--;face[2]--;
            // Adjust indices to be zero-based
            faces.push_back(face);
        }
    }
}

// Draw triangles in opencv
void drawTriangle(cv::Mat &image, const cv::Point2f &v1, const cv::Point2f &v2, const cv::Point2f &v3) {
    // Example projection from 3D to 2D (this might need adjustments)
    

    line(image, v1, v2, cv::Scalar(255, 0, 0), 1);
    line(image, v2, v3, cv::Scalar(255, 0, 0), 1);
    line(image, v3, v1, cv::Scalar(255, 0, 0), 1);
}

