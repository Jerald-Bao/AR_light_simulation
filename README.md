# AR_light_estimation
an Augmented Reality (AR) application designed to replicate the real-world lighting environment captured in video, enhancing realism by using deep learning and computer graphics techniques.

## Markless pattern detector and Pose Calculation System
The system uses SIFT for the feature detector, as well as feature extraction, and Flann-based matcher for feature matching.
Acknowledge: [https://github.com/MasteringOpenCV/code/tree/master](https://github.com/MasteringOpenCV/code)

## Argumented Reality rendering

The rendering component generates a highly realistic AR experience by simulating a single-model, single-light-source scene using OpenGL. The application integrates libraries such as glm for mathematics, glfw for window creation and management, and glad for OpenGL context management. After determining the reference pattern's pose, the system transforms it into the camera's view matrix, allowing seamless interaction between virtual objects and the real-world environment.

## Light estimation

The application enhances AR realism by analyzing video frames through a pre-trained CNN, which estimates one of eight possible light source directions relative to the camera's perspective. This real-time light direction estimation significantly improves the lighting consistency between the AR objects and their real-world surroundings. The light estimation model was trained using Python and integrates efficiently with the rendering pipeline.
