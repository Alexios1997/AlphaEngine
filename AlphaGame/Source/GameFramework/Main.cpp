//#include "AlphaEngine/Source/Application.h"
#include <glad/gl.h>      
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>    
#include <iostream>
#include "EngineFramework/Event.h"

int main() 
{
    if (!glfwInit()) return -1;

    // Set OpenGL 4.6 Core Profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "2026 OpenGL Test", NULL, NULL);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);

    // Initialize GLAD
    if (!gladLoadGL(glfwGetProcAddress)) return -1;

    // Test GLM (Mathematics)
    glm::vec3 testVec(1.0f, 2.0f, 3.0f);
    std::cout << "GLM Test Vector: " << testVec.x << ", " << testVec.y << ", " << testVec.z << std::endl;

    while (!glfwWindowShouldClose(window)) {
        // Test OpenGL & GLAD (Clear color)
        glClearColor(0.2f, 0.3f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}