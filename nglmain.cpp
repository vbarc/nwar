#include <glm/ext.hpp>
#include <glm/glm.hpp>

#include <soloud.h>

#include "NglArmyLayer.h"
#include "NglBuffer.h"
#include "NglCamera.h"
#include "NglProgram.h"
#include "NglSoundGenerator.h"
#include "NglTerrainGeometry.h"
#include "NglTerrainLayer.h"
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
    mat4 model_view_matrix;
    mat4 projection_matrix;
    float time;
    int32_t is_wireframe_enabled;
};

static NglCamera gCamera(vec3(0.0f, 1.6f, 1.6f), vec3(0.0f, 0.6f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
static bool gIsWireFrameEnabled = false;

static void doMain(GLFWwindow* window) {
    glClearColor(0.4f, 0.6f, 1.0f, 1.0f);
    NGL_CHECK_ERRORS;
    glEnable(GL_DEPTH_TEST);
    NGL_CHECK_ERRORS;
    glEnable(GL_CULL_FACE);
    NGL_CHECK_ERRORS;
    glCullFace(GL_BACK);
    NGL_CHECK_ERRORS;

    NglProgram program = NglProgram::Builder()
                                 .setVertexShader(gVertexShaderSrc)
                                 .setGeometryShader(gGeometryShaderSrc)
                                 .setFragmentShader(gFragmentShaderSrc)
                                 .build();
    program.use();

    // FrameUniform
    NglBuffer frameUniformBuffer;
    int frameUniformSize = sizeof(FrameUniform);
    glNamedBufferStorage(frameUniformBuffer, frameUniformSize, nullptr, GL_DYNAMIC_STORAGE_BIT);
    NGL_CHECK_ERRORS;
    glBindBufferBase(GL_UNIFORM_BUFFER, 0 /*FrameUniform*/, frameUniformBuffer);
    NGL_CHECK_ERRORS;
    FrameUniform frameUniform;

    // Layers
    NglTerrainGeometry terrainGeometry;
    NglTerrainLayer terrainLayer(terrainGeometry);
    NglArmyLayer armyLayer(terrainGeometry);

    int frameCounter = 0;
    double frameCounterStartTime = glfwGetTime();

    while (!glfwWindowShouldClose(window)) {
        double time = glfwGetTime();
        gCamera.onNextFrame(time);

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);
        NGL_CHECK_ERRORS;
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        NGL_CHECK_ERRORS;

        // FrameUniform
        frameUniform.model_view_matrix = gCamera.getModelViewMatrix();
        frameUniform.projection_matrix = glm::perspective(45.0f, width / static_cast<float>(height), 0.1f, 1000.0f);
        frameUniform.time = static_cast<float>(time);
        frameUniform.is_wireframe_enabled = gIsWireFrameEnabled ? 1 : 0;
        glBufferSubData(GL_UNIFORM_BUFFER, 0, frameUniformSize, &frameUniform);

        // Layers
        terrainLayer.draw();
        armyLayer.draw();

        double frameCounterWindow = time - frameCounterStartTime;
        if (frameCounterWindow >= 2) {
            int fps = (int)(frameCounter / frameCounterWindow);
            NGL_LOGI("FPS: %d", fps);
            frameCounterStartTime = time;
            frameCounter = 0;
        }
        frameCounter++;

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

int nglMain() {
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

    GLFWwindow* window = glfwCreateWindow(1920, 1080, "N War (OpenGL)", nullptr, nullptr);
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
        if (key == GLFW_KEY_SPACE && action != GLFW_RELEASE) {
            gIsWireFrameEnabled = !gIsWireFrameEnabled;
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

    NglSoundGenerator soundGenerator;

    doMain(window);

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
