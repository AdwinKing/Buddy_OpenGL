// Include standard headers
#include <iostream>
#include <fstream>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
#include "utils/shader.hpp"
//#include "utils/tinyply.h"

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const double MOUSE_SENSITIVITY = 2500;

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
    size_t count_far = 0;
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
    for (unsigned int i = 0; i < data->numOfFaces; i++) {
        glm::vec3 v1(vertices[data->faces[i * 3] * 6], vertices[data->faces[i * 3] * 6 + 1], vertices[data->faces[i * 3] * 6 + 2]);
        glm::vec3 v2(vertices[data->faces[i * 3 + 1] * 6], vertices[data->faces[i * 3 + 1] * 6 + 1], vertices[data->faces[i * 3 + 1] * 6 + 2]);
        glm::vec3 v3(vertices[data->faces[i * 3 + 2] * 6], vertices[data->faces[i * 3 + 2] * 6 + 1], vertices[data->faces[i * 3 + 2] * 6 + 2]);
        glm::vec3 a = v2 - v1;
        glm::vec3 b = v3 - v1;
        glm::vec3 normal = glm::cross(a, b);
        for (int j = 0; j < 3; j++) {
            vertices[data->faces[i * 3 + j] * 3] += normal[0];
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
//    for (size_t i = 0; i < data->numOfFaces; i++) {
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
    Shader cubeShader = Shader("/home/adwin/Codes/c++/opengl_tutorials/Learn_opengl/vertex.vert", "/home/adwin/Codes/c++/opengl_tutorials/Learn_opengl/frag.frag");
    Shader bunnyShader = Shader("/home/adwin/Codes/c++/opengl_tutorials/Learn_opengl/bunny.vert", "/home/adwin/Codes/c++/opengl_tutorials/Learn_opengl/bunny.frag");
    Shader lampShader = Shader("/home/adwin/Codes/c++/opengl_tutorials/Learn_opengl/lamp.vert", "/home/adwin/Codes/c++/opengl_tutorials/Learn_opengl/lamp.frag");

    //prepare vertex buffer
    unsigned int VAO1, VBO1, EBO1, VAO2, VBO2, EBO2;
    PLYdata* bunnyData = loadPLYData("/home/adwin/Codes/c++/opengl_tutorials/Learn_opengl/bunny_iH.ply2");
//    for (unsigned int i = 0; i < bunnyData->numOfVertices * 3; i++) {
//        bunnyData->vertices[i] /= 10;
//    }
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

    glm::vec3 lampPosition = glm::vec3(4.0f, 4.0f, 0.0f);
    glm::vec3 lampColor = glm::vec3(1.0f, 1.0f, 1.0f);
    glm::vec3 bunnyColor = glm::vec3(0.8f, 0.8f, 0.6f);

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
        //std::cout << cameraPosition[0] << ";" << cameraPosition[1] << ";" << cameraPosition[2] << std::endl;

//        view = glm::rotate(glm::mat4(1.0f), -1.0f * (float)cameraHorizontalAngle, glm::vec3(0.0f, 1.0f, 0.0f));
//        view = glm::translate(view, -1.0f * cameraPosition);
//        view = glm::rotate(view, (float)(cameraVerticalAngle - M_PI / 2), glm::vec3(1.0f, 0.0f, 0.0f));
          view = glm::lookAt(cameraPosition, cameraPosition + glm::vec3((float)sin(cameraVerticalAngle) * sin(cameraHorizontalAngle), (float)cos(cameraVerticalAngle), (float)sin(cameraVerticalAngle) * cos(cameraHorizontalAngle)), glm::vec3(0.0f, 1.0f, 0.0f));

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
        lampPosition = 200.0f * glm::vec3((float)cos(glfwGetTime()), 0.0f, (float)sin(glfwGetTime()));
        lampShader.use();
        lampShader.setMat4("model", glm::translate(glm::scale(glm::mat4(1.0f), glm::vec3(0.1f, 0.1f, 0.1f)), lampPosition));
        lampShader.setMat4("view", view);
        lampShader.setMat4("projection", projection);
        lampShader.setVec3("lightColor", lampColor);
        glBindVertexArray(lightVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        bunnyShader.use();
        //bunnyShader.setMat4("model", glm::scale(glm::mat4(1.0f), glm::vec3(0.1f, 0.1f, 0.1f)));
        bunnyShader.setMat4("model", glm::mat4(1.0f));
        bunnyShader.setMat4("view", view);
        bunnyShader.setMat4("projection", projection);
        bunnyShader.setVec3("lightColor", lampColor);
        bunnyShader.setVec3("bunnyColor", bunnyColor);
        bunnyShader.setVec3("lightPos", lampPosition);
        bunnyShader.setVec3("cameraPos", cameraPosition);
        glBindVertexArray(VAO2);
        glDrawElements(GL_TRIANGLES, bunnyData->numOfFaces * 3, GL_UNSIGNED_INT, (void*)0);
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

