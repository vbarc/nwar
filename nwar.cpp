#include <stdio.h>
#include <stdlib.h>

#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <vector>

#include "NglCamera.h"
#include "NglProgram.h"
#include "ngldbg.h"
#include "nglerr.h"
#include "nglfrag.h"
#include "nglgl.h"
#include "ngllog.h"
#include "nglvert.h"

using glm::mat4;
using glm::vec3;

struct FrameUniform {
    mat4 mvp;
};

static const vec3 vertices[] = {
        vec3(-0.5f, -0.4f, 0.0f), vec3(0.0f, 0.5f, 0.0f),  vec3(0.5f, -0.4f, 0.0f),  //
        vec3(0.0f, 0.5f, 0.0f),   vec3(1.0f, 0.5f, 0.0f),  vec3(0.5f, -0.4f, 0.0f),  //
        vec3(0.5f, -0.4f, 0.0f),  vec3(1.0f, 0.5f, 0.0f),  vec3(1.5f, -0.4f, 0.0f),  //
        vec3(-0.5f, -0.4f, 0.0f), vec3(0.5f, -0.4f, 0.0f), vec3(0.0f, -1.4f, 0.0f),  //
        vec3(0.0f, -1.4f, 0.0f),  vec3(0.5f, -0.4f, 0.0f), vec3(1.0f, -1.4f, 0.0f),  //
        vec3(1.0f, -1.4f, 0.0f),  vec3(0.5f, -0.4f, 0.0f), vec3(1.5f, -0.4f, 0.0f),  //
};

NglCamera gCamera(vec3(1.0f, 1.0f, 2.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));

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

    // FrameUniform
    int frameUniformSize = sizeof(FrameUniform);
    GLuint frameUniformBuffer;
    glCreateBuffers(1, &frameUniformBuffer);
    NGL_CHECK_ERRORS;
    glNamedBufferStorage(frameUniformBuffer, frameUniformSize, nullptr, GL_DYNAMIC_STORAGE_BIT);
    NGL_CHECK_ERRORS;
    glBindBufferBase(GL_UNIFORM_BUFFER, 0 /*FrameUniform*/, frameUniformBuffer);
    NGL_CHECK_ERRORS;

    // Vertices
    GLuint vao;
    glCreateVertexArrays(1, &vao);
    NGL_CHECK_ERRORS;
    glBindVertexArray(vao);
    NGL_CHECK_ERRORS;

    int verticesSize = sizeof(vertices);
    int verticesCount = static_cast<int>(std::size(vertices));
    std::vector<vec3> vertexBufferContent(verticesCount * 2);
    memcpy(vertexBufferContent.data(), vertices, verticesSize);
    for (int i = 0; i < verticesCount; i += 3) {
        vertexBufferContent[verticesCount + i + 0] = vec3(1.0f, 0.0f, 0.0f);
        vertexBufferContent[verticesCount + i + 1] = vec3(0.0f, 1.0f, 0.0f);
        vertexBufferContent[verticesCount + i + 2] = vec3(0.0f, 0.0f, 1.0f);
    }

    GLuint vertexBuffer;
    glCreateBuffers(1, &vertexBuffer);
    NGL_CHECK_ERRORS;
    glNamedBufferStorage(vertexBuffer, vertexBufferContent.size() * sizeof(vec3), vertexBufferContent.data(), 0);
    NGL_CHECK_ERRORS;

    glVertexArrayAttribFormat(vao, 0 /*pos*/, 3, GL_FLOAT, GL_FALSE, 0);
    NGL_CHECK_ERRORS;
    glVertexArrayAttribBinding(vao, 0, 0);
    NGL_CHECK_ERRORS;
    glVertexArrayVertexBuffer(vao, 0, vertexBuffer, 0, sizeof(vec3));
    NGL_CHECK_ERRORS;
    glEnableVertexArrayAttrib(vao, 0);
    NGL_CHECK_ERRORS;

    glVertexArrayAttribFormat(vao, 1 /*barypos_in*/, 3, GL_FLOAT, GL_FALSE, 0);
    NGL_CHECK_ERRORS;
    glVertexArrayAttribBinding(vao, 1, 1);
    NGL_CHECK_ERRORS;
    glVertexArrayVertexBuffer(vao, 1, vertexBuffer, verticesSize, sizeof(vec3));
    NGL_CHECK_ERRORS;
    glEnableVertexArrayAttrib(vao, 1);
    NGL_CHECK_ERRORS;

    glClearColor(0.4f, 0.6f, 1.0f, 1.0f);
    NGL_CHECK_ERRORS;

    FrameUniform frameUniform;

    while (!glfwWindowShouldClose(window)) {
        gCamera.onNextFrame();

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);
        NGL_CHECK_ERRORS;
        glClear(GL_COLOR_BUFFER_BIT);
        NGL_CHECK_ERRORS;

        mat4 v = gCamera.getViewMatrix();
        mat4 p = glm::perspective(45.0f, width / static_cast<float>(height), 0.1f, 1000.0f);
        frameUniform.mvp = p * v;
        glBufferSubData(GL_UNIFORM_BUFFER, 0, frameUniformSize, &frameUniform);

        glDrawArrays(GL_TRIANGLES, 0, verticesCount);
        NGL_CHECK_ERRORS;

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteBuffers(1, &vertexBuffer);
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &frameUniformBuffer);

    glfwDestroyWindow(window);
    glfwTerminate();

    // TODO: Indexed draw
    // TODO: Landscape
    // TODO: Skybox
    // TODO: Blender model
    // TODO: Nice rendering (lighting, grass material, cloth material?)
    // TODO: Animation
    return 0;
}
