#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>

#include "nglerr.h"

static const char* vertexShaderSrc = R"(
#version 460 core

layout (location=0) in vec3 pos;

void main() {
    gl_Position = vec4(pos, 1);
}
)";

static const char* fragmentShaderSrc = R"(
#version 460 core

layout (location=0) out vec4 color;

void main() {
    color = vec4(1, 0, 0, 1);
}
)";

static const float vertices[] = {
        -0.5f, -0.4f, 0.0f, 0.0f, 0.5f, 0.0f, 0.5f, -0.4f, 0.0f,
};

int main(void) {
    glfwSetErrorCallback([](int error, const char* description) { fprintf(stderr, "Error: %s\n", description); });

    if (!glfwInit()) {
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1920, 1080, "N War", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
    });

    glfwMakeContextCurrent(window);
    gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress));
    glfwSwapInterval(1);

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    NGLCHKERR;
    glShaderSource(vertexShader, 1, &vertexShaderSrc, nullptr);
    NGLCHKERR;
    glCompileShader(vertexShader);
    NGLCHKERR;

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    NGLCHKERR;
    glShaderSource(fragmentShader, 1, &fragmentShaderSrc, nullptr);
    NGLCHKERR;
    glCompileShader(fragmentShader);
    NGLCHKERR;

    GLuint program = glCreateProgram();
    NGLCHKERR;
    glAttachShader(program, vertexShader);
    NGLCHKERR;
    glAttachShader(program, fragmentShader);
    NGLCHKERR;
    glLinkProgram(program);
    NGLCHKERR;
    glUseProgram(program);
    NGLCHKERR;

    GLuint vao;
    glCreateVertexArrays(1, &vao);
    NGLCHKERR;
    glBindVertexArray(vao);
    NGLCHKERR;

    GLuint vertexBuffer;
    glGenBuffers(1, &vertexBuffer);
    NGLCHKERR;
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    NGLCHKERR;
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    NGLCHKERR;
    glEnableVertexAttribArray(0 /*pos*/);
    NGLCHKERR;
    glVertexAttribPointer(0 /*pos*/, 3, GL_FLOAT, GL_FALSE, 0, static_cast<void*>(0));
    NGLCHKERR;

    glClearColor(0.4f, 0.6f, 1.0f, 1.0f);
    NGLCHKERR;

    while (!glfwWindowShouldClose(window)) {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);
        NGLCHKERR;
        glClear(GL_COLOR_BUFFER_BIT);
        NGLCHKERR;

        glDrawArrays(GL_TRIANGLES, 0, 3);
        NGLCHKERR;

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    // TODO: Add robust error checking
    // TODO: Logging macros
    // TODO: Helpers for shaders and program
    // TODO: Destroy everything
    // TODO: MVP
    // TODO: Multiple triangles
    // TODO: Wireframe
    // TODO: Landscape
    return 0;
}
