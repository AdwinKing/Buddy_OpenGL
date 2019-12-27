// Include standard headers
#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
#include "utils/shader.hpp"
//#include "utils/tinyply.h"

const int WINDOW_WIDTH = 1600;
const int WINDOW_HEIGHT = 1200;
const double MOUSE_SENSITIVITY = 2500;
const float SELECTION_RANGE = 20.0f;

struct CallbackContext {
    double preXpos;
    double preYpos;
    glm::vec3 *cameraPos;
    double *cameraHorizontalAngle;
    double *cameraVerticalAngle;
};

struct PLYdata {
    unsigned int numOfVertices;
    unsigned int numOfFaces;
    float* vertices;
    unsigned int* faces;
};

PLYdata* loadPLYData(const char* filepath)
{
    PLYdata* data = new PLYdata;
    std::ifstream infile(filepath);
    infile >> data->numOfVertices >> data->numOfFaces;
    std::cout << data->numOfVertices << ";" << data->numOfFaces << std::endl;
//    data->vertices = new float[data->numOfVertices * 3];
    float* vertices = new float[data->numOfVertices * 6];
    long count_far = 0;
    for (unsigned int i = 0; i < data->numOfVertices; i++) {
        infile >> vertices[i * 6] >> vertices[i * 6 + 1] >> vertices[i * 6 + 2];
        if (vertices[i * 6] * vertices[i * 6] + vertices[i * 6 + 1] * vertices[i * 6 + 1] + vertices[i * 6 + 2] * vertices[i * 6 + 2] > 400){
            std::cout << vertices[i * 6] << ";" << vertices[i * 6 + 1] << ";" << vertices[i * 6 + 2] << std::endl;
            count_far++;
        }
        vertices[i * 6 + 3] = vertices[i * 6 + 4] = vertices[i * 6 + 5] = 0.0f;
    }
    std::cout << "Number of vertices that is far away:" << count_far << std::endl;
    data->faces = new unsigned int[data->numOfFaces * 3];
    char c;
    for (unsigned int i = 0; i < data->numOfFaces; i++){
        infile >> c >> data->faces[i * 3] >> data->faces[i * 3 + 1] >> data->faces[i * 3 + 2];
    }
    // calculate normal vector for each vertex
    for (unsigned int i = 0; i < data->numOfFaces; i++) {
        glm::vec3 v1(vertices[data->faces[i * 3] * 6], vertices[data->faces[i * 3] * 6 + 1], vertices[data->faces[i * 3] * 6 + 2]);
        glm::vec3 v2(vertices[data->faces[i * 3 + 1] * 6], vertices[data->faces[i * 3 + 1] * 6 + 1], vertices[data->faces[i * 3 + 1] * 6 + 2]);
        glm::vec3 v3(vertices[data->faces[i * 3 + 2] * 6], vertices[data->faces[i * 3 + 2] * 6 + 1], vertices[data->faces[i * 3 + 2] * 6 + 2]);
        glm::vec3 a = v2 - v1;
        glm::vec3 b = v3 - v1;
        glm::vec3 normal = glm::normalize(glm::cross(a, b));
        //glm::vec3 normal = glm::cross(a, b);
        for (int j = 0; j < 3; j++) {
            vertices[data->faces[i * 3 + j] * 6 + 3] += normal[0];
            vertices[data->faces[i * 3 + j] * 6 + 4] += normal[1];
            vertices[data->faces[i * 3 + j] * 6 + 5] += normal[2];
        }
    }
//    for (int i = 0; i < 36; i++) {
//        std::cout << vertices[i] << std::endl;
//    }

//    count_far = 0;
//    for (unsigned int i = 0; i < data->numOfVertices; i++) {
//        if (vertices[i * 6 + 3] * vertices[i * 6 + 3] + vertices[i * 6 + 4] * vertices[i * 6 + 4] + vertices[i * 6 + 5] * vertices[i * 6 + 5] > 100){
//            std::cout << vertices[i * 6 + 3] << ";" << vertices[i * 6 + 4] << ";" << vertices[i * 6 + 5] << std::endl;
//            count_far++;
//        }
//    }
//    std::cout << "Number of normal vectors that is of big magnitude:" << count_far << std::endl;

//    data->numOfFaces -= 69449;
//    for (long i = 0; i < data->numOfFaces; i++) {
//        for (int j = 0; j < 3; j++) {
//            for (int k = 0; k < 3; k++) {
//                std::cout << vertices[data->faces[i * 3 + j] + k] << ',';
//            }
//            std::cout << ';';
//        }
//        std::cout << std::endl;
//    }

    data->vertices = vertices;
    return data;

}

float rayTriangleIntersect(const glm::vec3 &orig, const glm::vec3 &dir, const glm::vec3 &v0, const glm::vec3 &v1, const glm::vec3 &v2)
{
    // compute plane's normal
    glm::vec3 v0v1 = v1 - v0;
    glm::vec3 v1v2 = v2 - v1;
    // no need to normalize
    glm::vec3 N = glm::cross(v0v1, v1v2); // N
    float area2 = N.length();

    // Step 1: finding P

    // check if ray and plane are parallel ?
    float NdotRayDirection = glm::dot(N, dir);
    if (fabs(NdotRayDirection) < 0.01f) // almost 0
        return -1.0f; // they are parallel so they don't intersect !

    // compute d parameter using equation 2
    float d = -glm::dot(N, v0);

    // compute t (equation 3)
    float t = -(glm::dot(N, orig) + d) / NdotRayDirection;
    // check if the triangle is in behind the ray
    if (t < 0) return -1.0f; // the triangle is behind

    // compute the intersection point using equation 1
    glm::vec3 P = orig + t * dir;

    // Step 2: inside-outside test
    glm::vec3 C; // vector perpendicular to triangle's plane

    // edge 0
    glm::vec3 edge0 = v1 - v0;
    glm::vec3 vp0 = P - v0;
    C = glm::cross(edge0, vp0);
    if (glm::dot(N, C) < 0) return -1.0f; // P is on the right side

    // edge 1
    glm::vec3 edge1 = v2 - v1;
    glm::vec3 vp1 = P - v1;
    C = glm::cross(edge1, vp1);
    if (glm::dot(N, C) < 0)  return -1.0f; // P is on the right side

    // edge 2
    glm::vec3 edge2 = v0 - v2;
    glm::vec3 vp2 = P - v2;
    C = glm::cross(edge2, vp2);
    if (glm::dot(N, C) < 0) return -1.0f; // P is on the right side;

    return t; // this ray hits the triangle
}

//std::vector<long> getSelectedTriangleIndex(const PLYdata &data, const glm::vec3 &camPos, const glm::vec3 &dir)
//{
//    float distanceToCamera = SELECTION_RANGE * 2;
//    long indexOfNearest = -1;
//    float nearestDistance = distanceToCamera;
//    std::vector<long> result;
//    for (long i = 0; i < data.numOfFaces; i++) {
//        // filter out triangles that are too far
//        bool atLeastOneNear = false;
//        for (int j = 0; j < 3; j++) {
//            bool isNear = true;
//            for (int k =0; k < 3; k++) {
//                if (fabs(data.vertices[data.faces[i * 3 + j] * 6 + k] - camPos[k]) >  SELECTION_RANGE) {
//                    isNear = false;
//                    break;
//                }
//            }
//            if (isNear) {
//                atLeastOneNear = true;
//                break;
//            }
//        }
//        if (!atLeastOneNear) continue;
//        float distance = rayTriangleIntersect(camPos, dir, glm::vec3(data.vertices[data.faces[i * 3] * 6], data.vertices[data.faces[i * 3] * 6 + 1], data.vertices[data.faces[i * 3] * 6 + 2]), glm::vec3(data.vertices[data.faces[i * 3 + 1] * 6], data.vertices[data.faces[i * 3 + 1] * 6 + 1], data.vertices[data.faces[i * 3 + 1] * 6 + 2]), glm::vec3(data.vertices[data.faces[i * 3 + 2] * 6], data.vertices[data.faces[i * 3 + 2] * 6 + 1], data.vertices[data.faces[i * 3 + 2] * 6 + 2]));
//        if (distance < 0) continue;
////        if (distance < nearestDistance) {
////            nearestDistance = distance;
////            indexOfNearest = i;
////        }
//        result.push_back(i);
//    }
//    //return indexOfNearest;
//    return result;
//}

long getSelectedTriangleIndex(const PLYdata &data, const glm::vec3 &camPos, const glm::vec3 &dir)
{
    float distanceToCamera = SELECTION_RANGE * 2;
    long indexOfNearest = -1;
    float nearestDistance = distanceToCamera;
    for (long i = 0; i < data.numOfFaces; i++) {
        // filter out triangles that are too far
        bool atLeastOneNear = false;
        for (int j = 0; j < 3; j++) {
            bool isNear = true;
            for (int k =0; k < 3; k++) {
                if (fabs(data.vertices[data.faces[i * 3 + j] * 6 + k] - camPos[k]) >  SELECTION_RANGE) {
                    isNear = false;
                    break;
                }
            }
            if (isNear) {
                atLeastOneNear = true;
                break;
            }
        }
        if (!atLeastOneNear) continue;
        float distance = rayTriangleIntersect(camPos, dir, glm::vec3(data.vertices[data.faces[i * 3] * 6], data.vertices[data.faces[i * 3] * 6 + 1], data.vertices[data.faces[i * 3] * 6 + 2]), glm::vec3(data.vertices[data.faces[i * 3 + 1] * 6], data.vertices[data.faces[i * 3 + 1] * 6 + 1], data.vertices[data.faces[i * 3 + 1] * 6 + 2]), glm::vec3(data.vertices[data.faces[i * 3 + 2] * 6], data.vertices[data.faces[i * 3 + 2] * 6 + 1], data.vertices[data.faces[i * 3 + 2] * 6 + 2]));
        if (distance < 0) continue;
        if (distance < nearestDistance) {
            nearestDistance = distance;
            indexOfNearest = i;
        }
    }
    return indexOfNearest;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos)
{
    CallbackContext* context = static_cast<CallbackContext*>(glfwGetWindowUserPointer(window));
    //std::cout << xpos << ";" << ypos << std::endl;
    *(context->cameraHorizontalAngle) -=  (xpos - context->preXpos) / MOUSE_SENSITIVITY;
    *(context->cameraHorizontalAngle) = fmod(*(context->cameraHorizontalAngle), 2 * M_PI);
    context->preXpos = xpos;
    *(context->cameraVerticalAngle) += (ypos - context->preYpos) / MOUSE_SENSITIVITY;
    //*(context->cameraVerticalAngle) = fmod(*(context->cameraVerticalAngle), M_PI);
    if (*(context->cameraVerticalAngle) < 0) {
        *(context->cameraVerticalAngle) = 0;
    } else if (*(context->cameraVerticalAngle) > M_PI) {
        *(context->cameraVerticalAngle) = M_PI;
    }
    context->preYpos = ypos;
}

int test_main()
{
    glm::vec3 origin = glm::vec3(0.0f, 0.0f, 0.0f);
    float result0 = rayTriangleIntersect(origin, glm::vec3(0.0f, 0.5f, 0.0f), glm::vec3(-2.0f, 1.0f, -1.0f), glm::vec3(1.0f, 1.0f, -1.0f), glm::vec3(0.0f, 1.0f, 1.0f));
    std::cout << result0 << std::endl;
    return 0;
}

int main( void )
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, cursor_pos_callback);

    if (GLEW_OK != glewInit()){
        std::cout << "Failed to init GLEW" << std::endl;
        return -1;
    }
    glEnable(GL_DEPTH_TEST);

    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

    //prepare data
//    float vertices[] = {
//        -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f,
//         0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f,
//         0.0f,  0.5f, 0.0f, 0.0f, 0.0f, 1.0f
//    };
    float vertices[] = {
            -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 1.0f,
             0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,
             0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,
             0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,
            -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 1.0f,

            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f,
             0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,
             0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f,
             0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f,
            -0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 0.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f,

            -0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,
            -0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f,
            -0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,

             0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,
             0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,
             0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,
             0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,
             0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f,
             0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,

            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,
             0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 0.0f,
             0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,
             0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,

            -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,
             0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,
             0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,
             0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,
            -0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,
            -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f
        };
    //load shaders
    //Shader cubeShader = Shader("/home/adwin/Codes/c++/opengl_tutorials/Learn_opengl/vertex.vert", "/home/adwin/Codes/c++/opengl_tutorials/Learn_opengl/frag.frag");
    Shader bunnyShader = Shader("/home/adwin/Codes/c++/opengl_tutorials/Learn_opengl/bunny.vert", "/home/adwin/Codes/c++/opengl_tutorials/Learn_opengl/bunny.frag");
    Shader lampShader = Shader("/home/adwin/Codes/c++/opengl_tutorials/Learn_opengl/lamp.vert", "/home/adwin/Codes/c++/opengl_tutorials/Learn_opengl/lamp.frag");

    //prepare vertex buffer
    unsigned int VAO1, VBO1, EBO1, VAO2, VBO2, EBO2;
    PLYdata* bunnyData = loadPLYData("/home/adwin/Codes/c++/opengl_tutorials/Learn_opengl/bunny_iH.ply2");

    // Bind data of lamp
    glGenVertexArrays(1, &VAO1);
    glGenBuffers(1, &VBO1);
    glGenBuffers(1, &EBO1);

    glBindVertexArray(VAO1);
    glBindBuffer(GL_ARRAY_BUFFER, VBO1);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    // Bind data of bunny
    glGenVertexArrays(1, &VAO2);
    glGenBuffers(1, &VBO2);
    glGenBuffers(1, &EBO2);

    glBindVertexArray(VAO2);
    glBindBuffer(GL_ARRAY_BUFFER, VBO2);
    glBufferData(GL_ARRAY_BUFFER, bunnyData->numOfVertices * 6 * sizeof(float), bunnyData->vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO2);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, bunnyData->numOfFaces * 3 * sizeof(unsigned int), bunnyData->faces, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    // note that this is allowed, the call to glVertexAttribPointer registered VBO1 as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind

    GLuint lightVAO;
    glGenVertexArrays(1, &lightVAO);
    glBindVertexArray(lightVAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO1);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // You can unbind the VAO1 afterwards so other VAO1 calls won't accidentally modify this VAO1, but this rarely happens. Modifying other
    // VAO1s requires a call to glBindVertexArray anyways so we generally don't unbind VAO1s (nor VBO1s) when it's not directly necessary.
    //glBindVertexArray(0);

    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 100.0f);
    glm::mat4 view = glm::mat4(1.0f);
    glm::vec3 cameraPosition = glm::vec3(0.0f, 0.0f, -3.0f);
    double cameraHorizontalAngle = 0.0;
    double cameraVerticalAngle = M_PI / 2;
    float cameraSpeed = 0.1f;
    CallbackContext glfwWindowContext = {WINDOW_WIDTH/2, WINDOW_HEIGHT/2, &cameraPosition, &cameraHorizontalAngle, &cameraVerticalAngle};
    glfwSetWindowUserPointer(window, &glfwWindowContext);

    glm::mat4 model = glm::mat4(1.0f);

    glm::vec3 lampPosition = glm::vec3(0.0f, 6.0f, 0.0f);
    glm::vec3 lampColor = glm::vec3(1.0f, 1.0f, 1.0f);
    glm::vec3 bunnyColor = glm::vec3(0.8f, 0.8f, 0.6f);


    int fps = 0;
    while(!glfwWindowShouldClose(window))
    {
        if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_ESCAPE))
            glfwSetWindowShouldClose(window, true);
        if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_W)){
            cameraPosition += glm::vec3(cameraSpeed * sin(cameraHorizontalAngle), 0.0f, cameraSpeed * cos(cameraHorizontalAngle));
        }
        if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_S)){
            cameraPosition -= glm::vec3(cameraSpeed * sin(cameraHorizontalAngle), 0.0f, cameraSpeed * cos(cameraHorizontalAngle));
        }
        if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_A)){
            cameraPosition += glm::vec3(cameraSpeed * cos(cameraHorizontalAngle), 0.0f, -cameraSpeed * sin(cameraHorizontalAngle));
        }
        if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_D)){
            cameraPosition -= glm::vec3(cameraSpeed * cos(cameraHorizontalAngle), 0.0f, -cameraSpeed * sin(cameraHorizontalAngle));
        }
        if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_SPACE)){
            cameraPosition += glm::vec3(0.0f, cameraSpeed, 0.0f);
        }
        if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_LEFT_SHIFT)){
            cameraPosition -= glm::vec3(0.0f, cameraSpeed, 0.0f);
        }
        if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_Q)){
            cameraHorizontalAngle += 0.05;
            cameraHorizontalAngle = fmod(cameraHorizontalAngle, M_PI * 2);
        }
        if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_E)){
            cameraHorizontalAngle -= 0.05;
            cameraHorizontalAngle = fmod(cameraHorizontalAngle, M_PI * 2);
        }
        //.
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        //.

//        glUseProgram(cubeProgramID);
//        glBindVertexArray(VAO1);
//        float timeValue = glfwGetTime();
//        float redTweak = sin(timeValue);
//        float greenTweak = sin(timeValue + 2.1f);
//        float blueTweak = sin(timeValue + 4.2f);
//        int colorTweakLocation = glGetUniformLocation(cubeProgramID, "colorTweak");
//        glUniform3f(colorTweakLocation, redTweak, greenTweak, blueTweak);
        if (fps % 100 == 0) {
            std::cout << cameraPosition[0] << ";" << cameraPosition[1] << ";" << cameraPosition[2] << std::endl;
            fps = 0;
        }
        fps++;

//        view = glm::rotate(glm::mat4(1.0f), -1.0f * (float)cameraHorizontalAngle, glm::vec3(0.0f, 1.0f, 0.0f));
//        view = glm::translate(view, -1.0f * cameraPosition);
//        view = glm::rotate(view, (float)(cameraVerticalAngle - M_PI / 2), glm::vec3(1.0f, 0.0f, 0.0f));
           glm::vec3 cameraDir = glm::vec3((float)sin(cameraVerticalAngle) * sin(cameraHorizontalAngle), (float)cos(cameraVerticalAngle), (float)sin(cameraVerticalAngle) * cos(cameraHorizontalAngle));
          view = glm::lookAt(cameraPosition, cameraPosition + cameraDir, glm::vec3(0.0f, 1.0f, 0.0f));

//        cubeShader.use();
//        glBindVertexArray(VAO1);
//        float timeValue = glfwGetTime();
//        float redTweak = sin(timeValue);
//        float greenTweak = sin(timeValue + 2.1f);
//        float blueTweak = sin(timeValue + 4.2f);
//        lampColor = glm::vec3(redTweak, greenTweak, blueTweak);
//        cubeShader.setVec3("colorTweak", glm::vec3(redTweak, greenTweak, blueTweak));
//        cubeShader.setMat4("model", glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 1.0f)));
//        cubeShader.setMat4("view", view);
//        cubeShader.setMat4("projection", projection);
//        glDrawArrays(GL_TRIANGLES, 0, 36);

//        view = glm::translate(glm::mat4(1.0f), cameraPosition);
//        view = glm::rotate(view, -1.0f * cameraHorizontalAngle, glm::vec3(0.0f, 1.0f, 0.0f));
//        int modelLocation = glGetUniformLocation(cubeProgramID, "model");
//        glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
//        int viewLocation = glGetUniformLocation(cubeProgramID, "view");
//        glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(view));
//        int projectionLocation = glGetUniformLocation(cubeProgramID, "projection");
//        glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projection));
        //glUniform3f(colorTweakLocation, 1.0f, 1.0f, 1.0f);
        //glBindVertexArray(VAO1);
        //glDrawArrays(GL_TRIANGLES, 0, 36);
        //.
        //lampPosition = 200.0f * glm::vec3((float)cos(glfwGetTime()), 0.0f, (float)sin(glfwGetTime()));
        //cameraPosition = 20.0f * glm::vec3((float)cos(glfwGetTime()), 0.0f, (float)sin(glfwGetTime()));
        //lampPosition = cameraPosition + glm::vec3(0.0f, 1.0f, 0.0f);
        //lampPosition = cameraPosition;
        lampShader.use();
        lampShader.setMat4("model", glm::translate(glm::scale(glm::mat4(1.0f), glm::vec3(0.2f, 0.2f, 0.2f)), 5.0f * lampPosition));
        //lampShader.setMat4("model", glm::translate(glm::mat4(1.0f), lampPosition));
        lampShader.setMat4("view", view);
        lampShader.setMat4("projection", projection);
        lampShader.setVec3("lightColor", lampColor);
        glBindVertexArray(lightVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);


        //draw bunny
        bunnyShader.use();
        bunnyShader.setMat4("model", glm::mat4(1.0f));
        bunnyShader.setMat4("view", view);
        bunnyShader.setMat4("projection", projection);
        bunnyShader.setVec3("lightColor", lampColor);
        bunnyShader.setVec3("bunnyColor", bunnyColor);
        bunnyShader.setVec3("lightPos", lampPosition);
        bunnyShader.setVec3("cameraPos", cameraPosition);
        glBindVertexArray(VAO2);

        long indexOfNearest = getSelectedTriangleIndex(*bunnyData, cameraPosition, cameraDir);
        if (indexOfNearest > 0) {
            glDrawElements(GL_TRIANGLES, 3 * indexOfNearest, GL_UNSIGNED_INT, (void*)0);
            bunnyShader.setVec3("bunnyColor", glm::vec3(0.8f, 0.0f, 0.0f));
            glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, (void*)(3 * indexOfNearest * sizeof(unsigned int)));
            bunnyShader.setVec3("bunnyColor", bunnyColor);
            glDrawElements(GL_TRIANGLES, 3 * (bunnyData->numOfFaces - indexOfNearest - 1), GL_UNSIGNED_INT, (void*)(3 * (indexOfNearest + 1) * sizeof(unsigned int)));
        } else {
            glDrawElements(GL_TRIANGLES, bunnyData->numOfFaces * 3, GL_UNSIGNED_INT, (void*)0);
        }
        //glDrawElements(GL_TRIANGLES, bunnyData->numOfFaces * 3, GL_UNSIGNED_INT, (void*)0);


        // draw selected triangle
//        long indexOfNearest = getSelectedTriangleIndex(*bunnyData, cameraPosition, cameraDir);
//        if (indexOfNearest > 0) {
//            bunnyShader.use();
//            //bunnyShader.setMat4("model", glm::scale(glm::mat4(1.0f), glm::vec3(0.1f, 0.1f, 0.1f)));
//            bunnyShader.setMat4("model", glm::mat4(1.0f));
//            bunnyShader.setMat4("view", view);
//            bunnyShader.setMat4("projection", projection);
//            bunnyShader.setVec3("lightColor", lampColor);
//            bunnyShader.setVec3("bunnyColor", glm::vec3(1.0f, 0.0f, 0.0f));
//            bunnyShader.setVec3("lightPos", lampPosition);
//            bunnyShader.setVec3("cameraPos", cameraPosition);
//            glBindVertexArray(VAO2);
//            glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, (void*)(3 * indexOfNearest * sizeof(unsigned int)));
//        }

        //long indexOfNearest = getSelectedTriangleIndex(*bunnyData, cameraPosition, cameraDir);
//        std::vector<long> indices = getSelectedTriangleIndex(*bunnyData, cameraPosition, cameraDir);

////        if (indexOfNearest > 0) {
//        if (indices.size() > 0) {
//            bunnyShader.use();
//            //bunnyShader.setMat4("model", glm::scale(glm::mat4(1.0f), glm::vec3(0.1f, 0.1f, 0.1f)));
//            bunnyShader.setMat4("model", glm::mat4(1.0f));
//            bunnyShader.setMat4("view", view);
//            bunnyShader.setMat4("projection", projection);
//            bunnyShader.setVec3("lightColor", lampColor);
//            bunnyShader.setVec3("bunnyColor", glm::vec3(1.0f, 0.0f, 0.0f));
//            bunnyShader.setVec3("lightPos", lampPosition);
//            bunnyShader.setVec3("cameraPos", cameraPosition);
//            glBindVertexArray(VAO2);
//            //glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, (void*)(3 * indexOfNearest * sizeof(unsigned int)));
//            for (auto const&i : indices) {
////                for (int j = 0; j < 3; j++) {
////                    std::cout << bunnyData->vertices[bunnyData->faces[i * 3 + j] * 6] << ',' << bunnyData->vertices[bunnyData->faces[i * 3 + j] * 6 + 1] << "," << bunnyData->vertices[bunnyData->faces[i * 3 + j] * 6 + 2] << "    ";
////                }
////                std::cout << std::endl;
//                glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, (void*)(3 * i * sizeof(unsigned int)));
//            }
//        }

        glfwSwapBuffers(window);
        glfwPollEvents();


    }

    glDeleteBuffers(1, &VBO1);
    glDeleteVertexArrays(1, &VAO1);
    glDeleteBuffers(1, &VBO2);
    glDeleteBuffers(1, &EBO2);
    glDeleteVertexArrays(1, &VAO2);
    glfwTerminate();
	return 0;
}

