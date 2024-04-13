/*
    Jerald Bao
    Spring 2024 CS 5330
    3/10/2024
    
    render.h
    rendering the virtual object using traditional openGL pipeline.
*/
#ifndef RENDER_H
#define RENDER_H
#include <opencv2/opencv.hpp>
#include <cstdio>
#include "glfw3.h"

/*
the main renderer pipeline of the opengl part
*/
void renderLoop(cv::Mat &img,cv::Mat& dst, std::vector<cv::Vec3f> v, std::vector<cv::Vec3i> indices,std::vector<cv::Vec3f> normals,
//cv::Mat& rvec,
float rmat[3][3],float tvec[],cv::Mat& perspective, GLFWwindow* window,GLint shaderProgram) ;

/*
compile and link shaders to the shader program
*/
void linkShader();

/*
set up the opengl context to use glw functions
*/
GLFWwindow* init_window();
/*
get the current shader program
*/
GLuint get_shader_program();
#endif