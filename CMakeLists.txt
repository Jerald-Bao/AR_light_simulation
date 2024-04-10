cmake_minimum_required(VERSION 3.28.1)

project(final_project)

if (APPLE)
    set (OpenCV_DIR /Users/draculasc2/Documents/libraries/install/lib/cmake/opencv4)
else()
    set (OpenCV_DIR C:/mylib/opencv/build)
    set(glfw3_DIR  C:/mylib/glfw-build/lib/cmake/glfw3)
endif(APPLE)
find_package(OpenCV REQUIRED)
find_package(OpenGL REQUIRED)
find_package(glfw3 3.4 REQUIRED)
include_directories(${glfw3_DIR})
include_directories(${OpenCV_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIRS} include/ C:/mylib/glfw-build/include/GLFW)
set (SOURCES
    src/vidDisplay.cpp
    src/calibration.cpp
    src/util.cpp
    src/render.cpp
    src/src/glad.c
    src/labelTool.cpp
    src/csv_util.cpp
)
add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE src  C:/mylib/glm ${OpenCV_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS} ${OPENGL_LIBRARIES} C:/mylib/glfw-build/lib/glfw3.lib)