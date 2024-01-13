#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <stdio.h>
#include <stdlib.h>

#include <assimp/Importer.hpp>
#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <vector>

#include "NglCamera.h"
#include "NglProgram.h"
#include "NglTerrain.h"
#include "nglassert.h"
#include "nglassimp.h"
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
    int32_t is_wireframe_enabled;
};

NglCamera gCamera(vec3(0.0f, 20.0f, 50.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
bool gIsWireFrameEnabled = false;

int main(void) {
    NglTerrain terrain;

    Assimp::Importer importer;

    const char* sceneFileName = "simple-man.glb";
    const aiScene* scene =
            importer.ReadFile(sceneFileName, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs |
                                                     aiProcess_JoinIdenticalVertices);
    if (scene) {
        NGL_LOGI("%s loaded successfully", sceneFileName);
    } else {
        NGL_LOGE("Error loading %s: %s", sceneFileName, importer.GetErrorString());
        abort();
    }

    std::vector<NglVertex> soldierVertices;
    std::vector<uint32_t> soldierIndices;
    for (unsigned int m = 0; m < scene->mNumMeshes; m++) {
        const aiMesh* mesh = scene->mMeshes[m];
        for (unsigned int v = 0; v < mesh->mNumVertices; v++) {
            NglVertex vertex;
            vertex.position = ai2glm(mesh->mVertices[v]);
            vertex.position += glm::vec3(0, 10, 0);  // Move up
            vertex.normal = ai2glm(mesh->mNormals[v]);
            soldierVertices.push_back(vertex);
        }
        for (unsigned int f = 0; f < mesh->mNumFaces; f++) {
            const aiFace& face = mesh->mFaces[f];
            NGL_ASSERT(face.mNumIndices == 3);
            soldierIndices.push_back(face.mIndices[0]);
            soldierIndices.push_back(face.mIndices[1]);
            soldierIndices.push_back(face.mIndices[2]);
        }
    }

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

    // Terrain

    // VAO
    GLuint terrainVao;
    glCreateVertexArrays(1, &terrainVao);
    NGL_CHECK_ERRORS;
    glBindVertexArray(terrainVao);
    NGL_CHECK_ERRORS;

    double startTime = glfwGetTime();
    std::vector<NglVertex> terrainVertices;
    std::vector<uint32_t> terrainIndices;
    terrain.getData(&terrainVertices, &terrainIndices);
    double terrainGenerationTime = glfwGetTime() - startTime;
    NGL_LOGI("Terrain generation time: %0.3fs", terrainGenerationTime);

    // Vertices
    GLuint terrainVertexBuffer;
    glCreateBuffers(1, &terrainVertexBuffer);
    NGL_CHECK_ERRORS;
    glNamedBufferStorage(terrainVertexBuffer, terrainVertices.size() * sizeof(NglVertex), terrainVertices.data(), 0);
    NGL_CHECK_ERRORS;

    glVertexArrayAttribFormat(terrainVao, 0 /*position*/, 3, GL_FLOAT, GL_FALSE, 0);
    NGL_CHECK_ERRORS;
    glVertexArrayAttribBinding(terrainVao, 0, 0);
    NGL_CHECK_ERRORS;
    glVertexArrayVertexBuffer(terrainVao, 0, terrainVertexBuffer, 0, sizeof(NglVertex));
    NGL_CHECK_ERRORS;
    glEnableVertexArrayAttrib(terrainVao, 0);
    NGL_CHECK_ERRORS;

    glVertexArrayAttribFormat(terrainVao, 1 /*normal*/, 3, GL_FLOAT, GL_FALSE, 0);
    NGL_CHECK_ERRORS;
    glVertexArrayAttribBinding(terrainVao, 1, 1);
    NGL_CHECK_ERRORS;
    glVertexArrayVertexBuffer(terrainVao, 1, terrainVertexBuffer, offsetof(NglVertex, normal), sizeof(NglVertex));
    NGL_CHECK_ERRORS;
    glEnableVertexArrayAttrib(terrainVao, 1);
    NGL_CHECK_ERRORS;

    // Indices
    GLuint terrainIndexBuffer;
    glCreateBuffers(1, &terrainIndexBuffer);
    NGL_CHECK_ERRORS;
    glNamedBufferStorage(terrainIndexBuffer, terrainIndices.size() * sizeof(uint32_t), terrainIndices.data(), 0);
    NGL_CHECK_ERRORS;
    glVertexArrayElementBuffer(terrainVao, terrainIndexBuffer);

    glBindVertexArray(0);

    // Soldier

    // VAO
    GLuint soldierVao;
    glCreateVertexArrays(1, &soldierVao);
    NGL_CHECK_ERRORS;
    glBindVertexArray(soldierVao);
    NGL_CHECK_ERRORS;

    // Vertices
    GLuint soldierVertexBuffer;
    glCreateBuffers(1, &soldierVertexBuffer);
    NGL_CHECK_ERRORS;
    glNamedBufferStorage(soldierVertexBuffer, soldierVertices.size() * sizeof(NglVertex), soldierVertices.data(), 0);
    NGL_CHECK_ERRORS;

    glVertexArrayAttribFormat(soldierVao, 0 /*position*/, 3, GL_FLOAT, GL_FALSE, 0);
    NGL_CHECK_ERRORS;
    glVertexArrayAttribBinding(soldierVao, 0, 0);
    NGL_CHECK_ERRORS;
    glVertexArrayVertexBuffer(soldierVao, 0, soldierVertexBuffer, 0, sizeof(NglVertex));
    NGL_CHECK_ERRORS;
    glEnableVertexArrayAttrib(soldierVao, 0);
    NGL_CHECK_ERRORS;

    glVertexArrayAttribFormat(soldierVao, 1 /*normal*/, 3, GL_FLOAT, GL_FALSE, 0);
    NGL_CHECK_ERRORS;
    glVertexArrayAttribBinding(soldierVao, 1, 1);
    NGL_CHECK_ERRORS;
    glVertexArrayVertexBuffer(soldierVao, 1, soldierVertexBuffer, offsetof(NglVertex, normal), sizeof(NglVertex));
    NGL_CHECK_ERRORS;
    glEnableVertexArrayAttrib(soldierVao, 1);
    NGL_CHECK_ERRORS;

    // Indices
    GLuint soldierIndexBuffer;
    glCreateBuffers(1, &soldierIndexBuffer);
    NGL_CHECK_ERRORS;
    glNamedBufferStorage(soldierIndexBuffer, soldierIndices.size() * sizeof(uint32_t), soldierIndices.data(), 0);
    NGL_CHECK_ERRORS;
    glVertexArrayElementBuffer(soldierVao, soldierIndexBuffer);

    glBindVertexArray(0);

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

        frameUniform.model_view_matrix = gCamera.getModelViewMatrix();
        frameUniform.projection_matrix = glm::perspective(45.0f, width / static_cast<float>(height), 0.1f, 1000.0f);
        frameUniform.is_wireframe_enabled = gIsWireFrameEnabled ? 1 : 0;
        glBufferSubData(GL_UNIFORM_BUFFER, 0, frameUniformSize, &frameUniform);

        glBindVertexArray(terrainVao);
        glDrawElements(GL_TRIANGLES, static_cast<int>(std::size(terrainIndices)), GL_UNSIGNED_INT, 0);
        NGL_CHECK_ERRORS;

        glBindVertexArray(soldierVao);
        glDrawElements(GL_TRIANGLES, static_cast<int>(std::size(soldierIndices)), GL_UNSIGNED_INT, 0);
        NGL_CHECK_ERRORS;

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteBuffers(1, &soldierIndexBuffer);
    glDeleteBuffers(1, &soldierVertexBuffer);
    glDeleteVertexArrays(1, &soldierVao);

    glDeleteBuffers(1, &terrainIndexBuffer);
    glDeleteBuffers(1, &terrainVertexBuffer);
    glDeleteVertexArrays(1, &terrainVao);

    glDeleteBuffers(1, &frameUniformBuffer);

    glfwDestroyWindow(window);
    glfwTerminate();

    // TODO: Blender model
    // TODO: Nice rendering (lighting, grass material, cloth material?)
    // TODO: Animation
    return 0;
}
