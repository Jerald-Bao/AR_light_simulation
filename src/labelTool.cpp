
#include "opencv2/opencv.hpp"
#include "csv_util.h"
#include <filesystem>
#include <windows.h>
#include <cstdlib>
#include <cstdio>
namespace lt{
int clicked;

void onMouse(int event, int x, int y, int flags, void* userdata) {
    if (event == cv::EVENT_LBUTTONDOWN) {
        // Define button area, for example, a rectangle where your "button" is
        cv::Rect2d rects[8];
        rects[0] = cv::Rect2d(209, 0, 208, 160); // x, y, width, height
        rects[1] = cv::Rect2d(417, 0, 208, 160); // x, y, width, height
        rects[2] = cv::Rect2d(417, 161, 208, 160); // x, y, width, height
        rects[3] = cv::Rect2d(417, 321, 208, 160); // x, y, width, height
        rects[4] = cv::Rect2d(209, 321, 208, 160); // x, y, width, height
        rects[5] = cv::Rect2d(0, 321, 208, 160); // x, y, width, height
        rects[6] = cv::Rect2d(0, 161, 208, 160); // x, y, width, height
        rects[7] = cv::Rect2d(0, 0, 208, 160); // x, y, width, height
        for (int i=0;i<8;i++){
            if (rects[i].contains(cv::Point2d(x, y))) {
                clicked = i;
            }
        }
    }
}

void showArrow(cv::Mat& src, cv::Mat& dst, int label) {
    src.copyTo(dst);
    cv::Point2d from,to;
    switch (label)
    {
    case 0:
        from = cv::Point2d(320,20);
        to = cv::Point2d(320,70);
        break;
    case 1:
        from = cv::Point2d(620,20);
        to = cv::Point2d(554,70);
        break;
    case 2:
        from = cv::Point2d(620,240);
        to = cv::Point2d(554,240);
        break;
    case 3:
        from = cv::Point2d(620,460);
        to = cv::Point2d(554,410);
        break;
    case 4:
        from = cv::Point2d(320,460);
        to = cv::Point2d(320,410);
        break;
    case 5:
        from = cv::Point2d(20,460);
        to = cv::Point2d(86,410);
        break;
    case 6:
        from = cv::Point2d(20,240);
        to = cv::Point2d(86,240);
        break;
    case 7:
        from = cv::Point2d(20,20);
        to = cv::Point2d(86,70);
        break;
    default:
        break;
    }
    cv::arrowedLine(dst,from,to,cv::Scalar(0,255,0),2);
    
}

int showImages(char* csv_filepath, char* img_dir) {
    cv::namedWindow("Label Image", cv::WINDOW_AUTOSIZE);
    std::vector<char*> img_names;
    std::vector<int> data;
    const char* img_name;
    read_image_data_csv(csv_filepath,img_names,data,false);

    WIN32_FIND_DATA findData;
    char path[100];
    strcpy(path,img_dir);
    strcat(path,"\\*");
    HANDLE hFind = FindFirstFile(path, &findData);
    // Set mouse callback
    cv::setMouseCallback("Label Image", onMouse);
    cv::moveWindow("Label Image",10,10);
    cv::Mat dst;
    std::vector<std::string> img_file_names;
    std::map<std::string, int> data_map;
    while (FindNextFile(hFind, &findData) != 0)
        if (strstr(findData.cFileName, ".jpg") ||
                strstr(findData.cFileName, ".png") ||
                strstr(findData.cFileName, ".ppm") ||
                strstr(findData.cFileName, ".tif"))
            {
                data_map.insert(std::make_pair(findData.cFileName, -1));
            }
    for (int i=0;i<img_names.size();i++)
    {
        data_map.insert_or_assign(img_names[i],data[i]);
    }
    std::map<std::string, int>::const_iterator it= data_map.begin();
    while (true) {
        if (it == data_map.end())
            it = data_map.begin();
        img_name = it -> first.c_str();
        char* full_path = new char[sizeof(img_name) + 11];
        strcpy(full_path,img_dir);
        strcat(full_path,img_name);
        cv::Mat img = cv::imread(full_path);
        if (img.empty())
            continue;
        cv::resize(img,img,cv::Size(640,480));
        img.copyTo(dst);
        showArrow(img, dst, it -> second);
        clicked = -1;
        bool exit = false;
        while (true) {
            cv::imshow("Label Image", dst);
            cv::setWindowTitle("Label Image",img_name);
            auto key = cv::waitKey(10);
            if (key == 27) {
                exit = true;
                break; // Exit on ESC
            } else if (key == 'd'){
                it++;
                if (it == data_map.end())
                    it = data_map.begin();
                break;
            } else if (key == 'a'){
                if (it == data_map.begin())
                    it = data_map.end();
                it--;
                break;
            }
            if (clicked != -1) {
                data_map[img_name] = clicked;
                break;
            }
        }
        if (exit)
            break;
    }
    bool first = true;
    for (const auto& pair : data_map) {
        append_image_data_csv(csv_filepath, pair.first.c_str(), pair.second,first);
        first = false;
    }
    return 0;
}
}