#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>

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
    glShaderSource(vertexShader, 1, &vertexShaderSrc, nullptr);
    glCompileShader(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSrc, nullptr);
    glCompileShader(fragmentShader);

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    glUseProgram(program);

    GLuint vao;
    glCreateVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint vertexBuffer;
    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0 /*pos*/);
    glVertexAttribPointer(0 /*pos*/, 3, GL_FLOAT, GL_FALSE, 0, static_cast<void*>(0));

    glClearColor(0.4f, 0.6f, 1.0f, 1.0f);

    while (!glfwWindowShouldClose(window)) {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);

        glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    // TODO: Add robust error checking
    // TODO: Helpers for shaders and program
    // TODO: Destroy everything
    // TODO: MVP
    // TODO: Multiple triangles
    // TODO: Wireframe
    // TODO: Landscape
    return 0;
}
