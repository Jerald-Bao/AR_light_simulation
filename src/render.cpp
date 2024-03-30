/*
    Jerald Bao
    Spring 2024 CS 5330
    3/10/2024
    
    render.cpp
    rendering the shaded object using traditional openGL pipeline.
*/
#include <cstdio>
#include <glad/glad.h>
#include "opencv2/opencv.hpp"
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <ext.hpp>
#include "render.h"
#include "glfw3.h"
static char* vertexShaderSource = R"glsl(
    #version 330 core
    layout(location=0) in vec3 position;
    layout(location=1) in vec3 normal;

    out vec3 normalOut;
    uniform mat4 viewMatrix;
    uniform mat4 projectionMatrix;
    uniform mat4 modelMatrix;


    void main() {
        gl_Position = projectionMatrix * viewMatrix *modelMatrix* vec4(position, 1.0);
        normalOut = normal;
        
    }
    )glsl";


static char* testVertexShaderSource = R"glsl(
    #version 330 core
    layout(location=0)in vec3 aPos;

    void main() {
        gl_Position =  vec4(aPos, 1.0);
    }
    )glsl";
const char* fragmentShaderSource = R"glsl(
    #version 330 core
    in vec3 normalOut;

    out vec4 color;

    void main() {
        vec3 normal = normalize(normalOut);
        vec3 lightDirection = normalize(vec3(0,0,-1));
        vec4 lightColor = vec4(1.0, 1.0, 1.0, 1.0);
        float ambientIntensity = 0.2;
        float lightIntensity = max(0,dot(normal, -lightDirection)) ;

        // clamp the light intensity to between 0 and 1

        // scale the light color to the light intensity
        lightIntensity = clamp(lightIntensity, 0, 1);
        lightIntensity += ambientIntensity;
        lightIntensity = clamp(lightIntensity, 0, 1);
        // multiply the color by the light intensity (after you get the texture value)

        color = vec4(vec3(1,1,1) * lightIntensity,1);
        //color = vec4(1,1,1,1);
    }
    )glsl";
static char* testFragmentShaderSource = R"glsl(
    #version 330 core
    out vec4 FragColor;
    void main() {
        FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
    }
    )glsl";

/*
compile a shader
*/
GLuint compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::" << (type == GL_VERTEX_SHADER ? "VERTEX" : "FRAGMENT") << "::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    return shader;
}

/*
get the current shader program
*/
GLuint get_shader_program() {
    GLuint vertex_shader_id;
    vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader_id, 1, &vertexShaderSource, NULL);
    glCompileShader(vertex_shader_id);

    GLint success;
    GLchar info_log[512];
    // Check if compilation was successful
    glGetShaderiv(vertex_shader_id, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertex_shader_id, 512, NULL, info_log);
        std::cout << "Fatal Error - Compilation of vertex shader failed:\n"
                  << info_log << std::endl;
        std::exit(-1);
    }

    GLuint fragment_shader_id;
    fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader_id, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragment_shader_id);

    // Link the vertex and fragment shader into a shader program
    GLuint shader_program_id;
    shader_program_id = glCreateProgram();
    glAttachShader(shader_program_id, vertex_shader_id);
    glAttachShader(shader_program_id, fragment_shader_id);
    glLinkProgram(shader_program_id);
    glUseProgram(shader_program_id);

    glGetShaderiv(fragment_shader_id, GL_COMPILE_STATUS, &success);
    // Same thing again
    if (!success) {
        glGetShaderInfoLog(fragment_shader_id, 512, NULL, info_log);
        std::cout << "Fatal Error - Compilation of fragment shader failed:\n"
                  << info_log << std::endl;
        std::exit(-1);
    }

    // Delete the individual shaders as they have been linked together and are
    // no longer needed separately
    glDeleteShader(vertex_shader_id);
    glDeleteShader(fragment_shader_id);

    return shader_program_id;
}

/*
convert opencv's camera matrix to opengl's projection matrix
*/
glm::mat4 convertCameraMatToPerspectiveMat(int width, int height, cv::Mat& cameraMat, float zNear, float zFar){
    glm::mat4 perspectiveMat = glm::mat4(0.0f);

    double fx = cameraMat.at<double>(0, 0);
    double fy = cameraMat.at<double>(1, 1);
    double cx = cameraMat.at<double>(0, 2);
    double cy = cameraMat.at<double>(1, 2);


    // Populate the matrix
    perspectiveMat[0][0] = 2 * fx / width;
    perspectiveMat[1][1] = 2 * fy / height;
    perspectiveMat[0][2] = (2 * cx - width) / width;
    perspectiveMat[1][2] = (2 * cy - height) / height;
    perspectiveMat[2][2] = -(zFar + zNear) / (zFar - zNear);
    perspectiveMat[2][3] = -1;
    perspectiveMat[3][2] = -(2 * zFar * zNear) / (zFar - zNear);
    return perspectiveMat;
}

/*
convert a cv::Mat to an opengl texture and bind it to the program
*/
GLuint matToTexture(const cv::Mat &mat, GLenum minFilter, GLenum magFilter, GLenum wrapFilter) {
    // Convert BGR to RGB
    cv::Mat rgbMat;
    cv::cvtColor(mat, rgbMat, cv::COLOR_BGR2RGB);

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapFilter);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, rgbMat.cols, rgbMat.rows, 0, GL_RGB, GL_UNSIGNED_BYTE, rgbMat.data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    return textureID;
}


/*
the main renderer pipeline of the opengl part
*/

void renderLoop(cv::Mat &img,cv::Mat& dst, std::vector<cv::Vec3f> v, std::vector<cv::Vec3i> indices,std::vector<cv::Vec3f> normals,
cv::Mat& rvec, cv::Mat& tvec,cv::Mat& perspective, GLFWwindow* window,GLint shaderProgram) {
    int width = 640;
    int height = 480;
    GLuint VBO, VAO, EBO;
    
    GLint num_attr;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &num_attr);

    glGetIntegerv(GL_CURRENT_PROGRAM, &shaderProgram);
    
    glClearColor(0.f,0.f,0.f, 0.0f); // Set clear color
    //glClear(GL_COLOR_BUFFER_BIT);   
    glViewport(0, 0, width, height);
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    // Bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

    size_t totalBytes = v.size() * sizeof(cv::Vec3f)*2;
    GLsizeiptr size = totalBytes / sizeof(float);
    float* vertexBufferData = new float[size];
    for (int i=0; i<size; i+=6) {
        vertexBufferData[i] = v[i/6][0];
        vertexBufferData[i+1] = v[i/6][1];
        vertexBufferData[i+2] = v[i/6][2];
        
        vertexBufferData[i+3] = normals[i/6][0];
        vertexBufferData[i+4] = normals[i/6][1];
        vertexBufferData[i+5] = normals[i/6][2];
    }
    glBufferData(GL_ARRAY_BUFFER, totalBytes, vertexBufferData, GL_STATIC_DRAW);
    //Binding indice buffer
    totalBytes = indices.size() * sizeof(cv::Vec3i);
    size = totalBytes / sizeof(unsigned int);
    unsigned int* indiceBufferData=new unsigned int[size];
    for (int i=0; i<size; i+=3) {
        indiceBufferData[i] = indices[i/3][0];
        indiceBufferData[i+1] = indices[i/3][1];
        indiceBufferData[i+2] = indices[i/3][2];
    }
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, totalBytes, indiceBufferData, GL_STATIC_DRAW);
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBindVertexArray(0);

    //set up lights
    // float lightPos[] = {1.2f, 1.0f, 2.0f};
    // int lightPosLocation = glGetUniformLocation(shaderProgram, "lightPos");
    // glUniform3f(lightPosLocation, lightPos[0], lightPos[1], lightPos[2]);

    // Calculate transformation matrices

/*
    To me in the future: DON'T TOUCH ALL THE CODES BELOW. I HAVE NO IDEA WHY THIS WORKS.
*/

    glm::vec4 cameraPos = glm::vec4((float)tvec.at<double>(0,0), (float)tvec.at<double>(1,0),(float)tvec.at<double>(2,0),0); // Camera position

    glm::mat4 viewMatrix = glm::mat4(1.0f);
    cv::Mat R;
    cv::Rodrigues(rvec, R);

    // Convert to a 4x4 transformation matrix
    cv::Mat T(4, 4, R.type(), cv::Scalar(0));
    R.copyTo(T(cv::Rect(0, 0, 3, 3)));
    // for (int i = 0; i < 3; ++i) {
    //     T.at<double>(i, 3) = tvec.at<double>(i,0);
    // }
    T.at<double>(3, 3) = 1.0;



    glm::mat4 testMatrix = glm::make_mat4(T.ptr<double>());
    glm::quat objRotationQua= glm::quat_cast(testMatrix);
    glm::vec3 objRotationEuler = glm::eulerAngles(objRotationQua); 
    std::cout<<"obj Rotation:" << objRotationEuler.x<<" "<<objRotationEuler.y<<" "<<objRotationEuler.z<<std::endl;

    glm::mat4 coordFixMatrix = glm::mat4(1.f);
    coordFixMatrix[1][1] = 1;
    glm::mat4 cameraRotation = glm::inverse(coordFixMatrix * testMatrix);
    glm::quat cameraRotationQua= glm::quat_cast(cameraRotation);
    glm::vec3 cameraRotationEuler = glm::eulerAngles(cameraRotationQua); 
    std::cout<<"camera Rotation:" << cameraRotationEuler.x<<" "<<cameraRotationEuler.y<<" "<<cameraRotationEuler.z<<std::endl;
    // {
    //     cv::Mat cvToGl = cv::Mat::zeros(4, 4, CV_64F);
    //     cvToGl.at<double>(0, 0) = -1.0f;
    //     cvToGl.at<double>(1, 1) = 1.0f; // Invert the y axis
    //     cvToGl.at<double>(2, 2) = 1.0f; // invert the z axis
    //     cvToGl.at<double>(3, 3) = 1.0f;
    //     T = cvToGl * T;
    //     cv::Mat glViewMatrix = cv::Mat::zeros(4, 4, CV_64F);
    //     cv::transpose(T , glViewMatrix);
    //     // Invert the matrix to get the view matrix
    //     glViewMatrix = glViewMatrix.inv();

    //     // Convert cv::Mat to glm::mat4 (if using GLM for OpenGL)
    //     viewMatrix = glm::make_mat4(glViewMatrix.ptr<double>());
        
    //     //viewMatrix = glm::translate(viewMatrix,glm::vec3(cameraPos));
    //     //viewMatrix = glm::mat4(1.0f);

    // }
    glm::vec4 translation = testMatrix * -cameraPos;

    // viewMatrix[3][0] = translation[0];
    // viewMatrix[3][1] = translation[1];
    // viewMatrix[3][2] = translation[2];

    cameraRotation = glm::translate(cameraRotation, glm::vec3(translation));
    std::cout<<"camera Pos:" << translation.x<<" "<<translation.y<<" "<<translation.z<<std::endl;
    glm::mat4 projection = convertCameraMatToPerspectiveMat(width,height,perspective, 0.1f, 100.0f);
    //glm::mat4 projection = glm::perspective<float>(glm::radians(40.), (float)640 / (float)480, 0.1f, 100.0f);

    GLint viewLoc = glGetUniformLocation(shaderProgram, "viewMatrix");

    // Pass the view matrix to the shader
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(cameraRotation));
    GLint projLoc = glGetUniformLocation(shaderProgram, "projectionMatrix");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
    glm::mat4  modelMatrix=  glm::mat4(1);
    modelMatrix = glm::scale(modelMatrix, glm::vec3(2) );
    GLint modelLoc = glGetUniformLocation(shaderProgram, "modelMatrix");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));

    GLuint backgroundTexture = matToTexture(img, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,backgroundTexture, 0);
    glBindTexture(GL_TEXTURE_2D, backgroundTexture);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, size, GL_UNSIGNED_INT, 0);
    //glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
    // Allocate memory for the pixel data (RGBA for each pixel)
    GLubyte* pixels = new GLubyte[4 * width * height];

    // Read the pixels from the frame buffer
    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    std::vector<GLubyte> rgbPixels(width * height * 3);
    for (int i = 0; i < width * height; ++i) {
        int rowCV = i / width;
        int rowGL = height - i / width -1;
        //int rowGL = i / width;
        int colCV = i % width;
        //int colGL = i % width;
        int colGL = width - i % width -1;
        int index = rowGL * width + colGL;
        if (pixels[4 * index + 3] >0.99)
        {
            rgbPixels[3 * i] =pixels[4 * index];     // R
            rgbPixels[3 * i + 1] = pixels[4 * index + 1]; // G
            rgbPixels[3 * i + 2] = pixels[4 * index + 2]; // B
        } else{
            rgbPixels[3 * i] = img.at<cv::Vec3b>(rowCV , colCV)[0];
            rgbPixels[3 * i + 1] = img.at<cv::Vec3b>(rowCV , colCV)[1]; 
            rgbPixels[3 * i + 2] = img.at<cv::Vec3b>(rowCV , colCV)[2];  
        }
    }
    
    // for (int i = 0; i < width * height; ++i) {
    //     int index = i;
    //     rgbPixels[3 * i] =pixels[4 * index];     // R
    //     rgbPixels[3 * i + 1] = pixels[4 * index + 1]; // G
    //     rgbPixels[3 * i + 2] = pixels[4 * index + 2]; // B
    // }
    // The pixel data in 'pixels' is upside down, and you might need to flip it
    // before saving, depending on how your image library handles pixel data.

    dst = cv::Mat(height, width, CV_8UC3,rgbPixels.data());
    //cv::flip(dst,dst,0);
    // Do something with the image
    // For example, display it
    cv::imshow("rendering", dst);
    //cv::waitKey(0);
    // Clean up
    delete[] pixels;
    // delete[] indiceBufferData;
    // delete[] vertexBufferData;
}


/*
set up the opengl context to use glw functions
*/
GLFWwindow *init_window() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    GLFWwindow *window =
        glfwCreateWindow(640, 480, "OpenGL Template", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        std::exit(-1);
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        std::exit(-1);
    }

    return window;
}
