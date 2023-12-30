#include <stdio.h>
#include <stdlib.h>

#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <vector>

#include "NglCamera.h"
#include "NglProgram.h"
#include "NglTerrain.h"
#include "ngldbg.h"
#include "nglerr.h"
#include "nglfrag.h"
#include "nglgeom.h"
#include "nglgl.h"
#include "ngllog.h"
#include "nglvert.h"

using glm::mat4;
using glm::vec3;

struct FrameUniform {
    mat4 mvp;
};

NglCamera gCamera(vec3(-20.0f, 15.0f, 50.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));

int main(void) {
    NglTerrain terrain;

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

    NglProgram program = NglProgram::Builder()
                                 .setVertexShader(gVertexShaderSrc)
                                 .setGeometryShader(gGeometryShaderSrc)
                                 .setFragmentShader(gFragmentShaderSrc)
                                 .build();
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

    // VAO
    GLuint vao;
    glCreateVertexArrays(1, &vao);
    NGL_CHECK_ERRORS;
    glBindVertexArray(vao);
    NGL_CHECK_ERRORS;

    std::vector<vec3> vertices;
    std::vector<uint32_t> indices;
    terrain.getData(&vertices, &indices);

    // Vertices
    GLuint vertexBuffer;
    glCreateBuffers(1, &vertexBuffer);
    NGL_CHECK_ERRORS;
    glNamedBufferStorage(vertexBuffer, vertices.size() * sizeof(vec3), vertices.data(), 0);
    NGL_CHECK_ERRORS;

    glVertexArrayAttribFormat(vao, 0 /*pos*/, 3, GL_FLOAT, GL_FALSE, 0);
    NGL_CHECK_ERRORS;
    glVertexArrayAttribBinding(vao, 0, 0);
    NGL_CHECK_ERRORS;
    glVertexArrayVertexBuffer(vao, 0, vertexBuffer, 0, sizeof(vec3));
    NGL_CHECK_ERRORS;
    glEnableVertexArrayAttrib(vao, 0);
    NGL_CHECK_ERRORS;

    // Indices
    GLuint indexBuffer;
    glCreateBuffers(1, &indexBuffer);
    NGL_CHECK_ERRORS;
    glNamedBufferStorage(indexBuffer, indices.size() * sizeof(uint32_t), indices.data(), 0);
    NGL_CHECK_ERRORS;
    glVertexArrayElementBuffer(vao, indexBuffer);

    glClearColor(0.4f, 0.6f, 1.0f, 1.0f);
    NGL_CHECK_ERRORS;
    glEnable(GL_DEPTH_TEST);
    NGL_CHECK_ERRORS;

    FrameUniform frameUniform;

    while (!glfwWindowShouldClose(window)) {
        gCamera.onNextFrame();

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);
        NGL_CHECK_ERRORS;
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        NGL_CHECK_ERRORS;

        mat4 v = gCamera.getViewMatrix();
        mat4 p = glm::perspective(45.0f, width / static_cast<float>(height), 0.1f, 1000.0f);
        frameUniform.mvp = p * v;
        glBufferSubData(GL_UNIFORM_BUFFER, 0, frameUniformSize, &frameUniform);

        glDrawElements(GL_TRIANGLES, static_cast<int>(std::size(indices)), GL_UNSIGNED_INT, 0);
        NGL_CHECK_ERRORS;

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteBuffers(1, &indexBuffer);
    glDeleteBuffers(1, &vertexBuffer);
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &frameUniformBuffer);

    glfwDestroyWindow(window);
    glfwTerminate();

    // TODO: Terrain
    // TODO: Skybox
    // TODO: Blender model
    // TODO: Nice rendering (lighting, grass material, cloth material?)
    // TODO: Animation
    return 0;
}
