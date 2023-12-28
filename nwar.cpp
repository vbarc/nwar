#include <stdio.h>
#include <stdlib.h>

#include <glm/ext.hpp>
#include <glm/glm.hpp>

#include "NglCamera.h"
#include "NglProgram.h"
#include "ngldbg.h"
#include "nglerr.h"
#include "nglfrag.h"
#include "nglgl.h"
#include "ngllog.h"
#include "nglvert.h"

static const float vertices[] = {
        -0.5f, -0.4f, 0.0f, 0.0f, 0.5f, 0.0f, 0.5f, -0.4f, 0.0f,
};

NglCamera gCamera(glm::vec3(1.0f, 1.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

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

    glfwSetKeyCallback(window, [](auto window, int key, int scancode, int action, int mods) {
        if (key == GLFW_KEY_ESCAPE && action != GLFW_RELEASE) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
            return;
        }
        if (gCamera.onKeyEvent(key, scancode, action, mods)) {
            return;
        }
    });

    glfwSetMouseButtonCallback(window, [](auto window, int button, int action, int mods) {
        if (gCamera.onMouseButtonEvent(window, button, action, mods)) {
            return;
        }
    });

    glfwSetCursorPosCallback(window, [](auto window, double x, double y) {
        if (gCamera.onMouseMotionEvent(window, x, y)) {
            return;
        }
    });

    glfwMakeContextCurrent(window);
    gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress));
    glfwSwapInterval(1);

    nglEnableDebugIfNecessary();

    NglProgram program =
            NglProgram::Builder().setVertexShader(gVertexShaderSrc).setFragmentShader(gFragmentShaderSrc).build();
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

    while (!glfwWindowShouldClose(window)) {
        gCamera.onNextFrame();

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
    // TODO: Modernize uniforms
    // TODO: Landscape
    // TODO: Skybox
    return 0;
}
