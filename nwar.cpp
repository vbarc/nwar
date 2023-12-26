#include <stdio.h>
#include <stdlib.h>

#include <glm/ext.hpp>
#include <glm/glm.hpp>

#include "NglCamera.h"
#include "NglProgram.h"
#include "ngldbg.h"
#include "nglerr.h"
#include "nglgl.h"
#include "ngllog.h"

static const char* vertexShaderSrc = R"(
#version 460 core

layout (location=0) in vec3 pos;
uniform mat4 mvp;

void main() {
    gl_Position = mvp * vec4(pos, 1);
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

NglCamera gCamera(glm::vec3(1.0f, 1.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

struct MouseState {
    glm::vec2 pos = glm::vec2(0.0f);
    bool pressedLeft = false;
} gMouseState;

int main(void) {
    glfwSetErrorCallback(
            [](int error, const char* description) { NGL_LOGE("GLFW error: %s (%d)", description, error); });

    if (!glfwInit()) {
        NGL_LOGE("glfwInit() failed");
        abort();
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    nglPrepareDebugIfNecessary();

    GLFWwindow* window = glfwCreateWindow(1920, 1080, "N War", nullptr, nullptr);
    if (!window) {
        NGL_LOGE("glfwCreateWindow() failed");
        glfwTerminate();
        abort();
    }

    glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
        const bool pressed = action != GLFW_RELEASE;
        if (key == GLFW_KEY_ESCAPE && pressed) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
        if (key == GLFW_KEY_W) {
            gCamera.movement_.forward_ = pressed;
        }
        if (key == GLFW_KEY_S) {
            gCamera.movement_.backward_ = pressed;
        }
        if (key == GLFW_KEY_A) {
            gCamera.movement_.left_ = pressed;
        }
        if (key == GLFW_KEY_D) {
            gCamera.movement_.right_ = pressed;
        }
        if (key == GLFW_KEY_1) {
            gCamera.movement_.up_ = pressed;
        }
        if (key == GLFW_KEY_2) {
            gCamera.movement_.down_ = pressed;
        }
        if (mods & GLFW_MOD_SHIFT) {
            gCamera.movement_.fastSpeed_ = pressed;
        }
        if (key == GLFW_KEY_SPACE) {
            gCamera.reset(glm::vec3(1.0f, 1.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        }
    });

    glfwSetMouseButtonCallback(window, [](auto* window, int button, int action, int mods) {
        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            gMouseState.pressedLeft = action == GLFW_PRESS;
        }
    });

    glfwSetCursorPosCallback(window, [](auto* window, double x, double y) {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        gMouseState.pos.x = static_cast<float>(x / width);
        gMouseState.pos.y = static_cast<float>(y / height);
    });

    glfwMakeContextCurrent(window);
    gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress));
    glfwSwapInterval(1);

    nglEnableDebugIfNecessary();

    NglProgram program =
            NglProgram::Builder().setVertexShader(vertexShaderSrc).setFragmentShader(fragmentShaderSrc).build();
    program.use();

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

    GLint mvpLocation = glGetUniformLocation(program, "mvp");

    glClearColor(0.4f, 0.6f, 1.0f, 1.0f);
    NGL_CHECK_ERRORS;

    double time = glfwGetTime();

    while (!glfwWindowShouldClose(window)) {
        const double newTime = glfwGetTime();
        float timeDelta = static_cast<float>(newTime - time);
        time = newTime;
        gCamera.update(timeDelta, gMouseState.pos, gMouseState.pressedLeft);

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);
        NGL_CHECK_ERRORS;
        glClear(GL_COLOR_BUFFER_BIT);
        NGL_CHECK_ERRORS;

        glm::mat4 v = gCamera.getViewMatrix();
        glm::mat4 p = glm::perspective(45.0f, width / static_cast<float>(height), 0.1f, 1000.0f);
        glm::mat4 mvp = p * v;
        glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, glm::value_ptr(mvp));

        glDrawArrays(GL_TRIANGLES, 0, 3);
        NGL_CHECK_ERRORS;

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteBuffers(1, &vertexBuffer);
    glDeleteVertexArrays(1, &vao);

    glfwDestroyWindow(window);
    glfwTerminate();

    // TODO: Multiple triangles
    // TODO: Wireframe
    // TODO: Landscape
    // TODO: Simplify camera
    return 0;
}
