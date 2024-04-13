# AR_light_estimation
An AR application that mimics the lighting environment of the video using a convolution neural network.

## Markless pattern detector and Pose Calculation System
The system uses SIFT for the feature detector, as well as feature extraction, and Flann-based matcher for feature matching.
Acknowledge: [https://github.com/MasteringOpenCV/code/tree/master](https://github.com/MasteringOpenCV/code)

## Argumented Reality rendering

The rendering part renders a single-model-single-light-source scene in OpenGL, using glm, glfw and glad libraries.
After the previous part calculates the pose of the reference pattern, transform it to get the camera's view matrix in OpenGL.

## Light estimation

the video frames are analyzed by a pre-trained CNN model and output one of eight light source directions in the image perspective. The training part is written in Python.
