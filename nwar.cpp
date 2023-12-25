#include <stdio.h>
#include <stdlib.h>

#include "ngldbg.h"
#include "nglerr.h"
#include "nglgl.h"
#include "ngllog.h"

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
    glfwSetErrorCallback([](int error, const char* description) { NGL_LOGE("GLFW error: %s", description); });

    if (!glfwInit()) {
        NGL_LOGE("glfwInit() failed");
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    nglPrepareDebugIfNecessary();

    GLFWwindow* window = glfwCreateWindow(1920, 1080, "N War", nullptr, nullptr);
    if (!window) {
        NGL_LOGE("glfwCreateWindow() failed");
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

    nglEnableDebugIfNecessary();

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    NGL_CHECK_ERRORS;
    glShaderSource(vertexShader, 1, &vertexShaderSrc, nullptr);
    NGL_CHECK_ERRORS;
    glCompileShader(vertexShader);
    NGL_CHECK_ERRORS;

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    NGL_CHECK_ERRORS;
    glShaderSource(fragmentShader, 1, &fragmentShaderSrc, nullptr);
    NGL_CHECK_ERRORS;
    glCompileShader(fragmentShader);
    NGL_CHECK_ERRORS;

    GLuint program = glCreateProgram();
    NGL_CHECK_ERRORS;
    glAttachShader(program, vertexShader);
    NGL_CHECK_ERRORS;
    glAttachShader(program, fragmentShader);
    NGL_CHECK_ERRORS;
    glLinkProgram(program);
    NGL_CHECK_ERRORS;
    glUseProgram(program);
    NGL_CHECK_ERRORS;

    GLuint vao;
    glCreateVertexArrays(1, &vao);
    NGL_CHECK_ERRORS;
    glBindVertexArray(vao);
    NGL_CHECK_ERRORS;

    GLuint vertexBuffer;
    glGenBuffers(1, &vertexBuffer);
    NGL_CHECK_ERRORS;
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    NGL_CHECK_ERRORS;
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    NGL_CHECK_ERRORS;
    glEnableVertexAttribArray(0 /*pos*/);
    NGL_CHECK_ERRORS;
    glVertexAttribPointer(0 /*pos*/, 3, GL_FLOAT, GL_FALSE, 0, static_cast<void*>(0));
    NGL_CHECK_ERRORS;

    glClearColor(0.4f, 0.6f, 1.0f, 1.0f);
    NGL_CHECK_ERRORS;

    while (!glfwWindowShouldClose(window)) {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);
        NGL_CHECK_ERRORS;
        glClear(GL_COLOR_BUFFER_BIT);
        NGL_CHECK_ERRORS;

        glDrawArrays(GL_TRIANGLES, 0, 3);
        NGL_CHECK_ERRORS;

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    // TODO: Helpers for shaders and program
    // TODO: Destroy everything
    // TODO: MVP
    // TODO: Multiple triangles
    // TODO: Wireframe
    // TODO: Landscape
    return 0;
}
