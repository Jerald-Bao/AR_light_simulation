/*
    Jerald Bao
    Spring 2024 CS 5330
    3/10/2024
    
    vidDisplay.cpp
    display a video from webcam and execute different functionalities activated by different keystrokes.
*/

#ifndef MAIN_H
#define MAIN_H
#include "opencv2/opencv.hpp"
namespace Mode{
/*
Enumerate const value for user interaction controls.
*/
const int origin = 1 << 0;
const int corners = 1 << 1;
const int save = 1 << 2;
const int calibration = 1 << 3;
const int tranfromation = 1 << 4;
const int render = 1 << 5;
const int renderGL = 1 << 7;
const int board = 1 << 6;
const int harris = 1 << 8;
const int project = 1 << 9;
}


#endif