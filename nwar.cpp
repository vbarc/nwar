#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <stdio.h>
#include <stdlib.h>

#include <assimp/Importer.hpp>
#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <vector>

#include "NglBuffer.h"
#include "NglCamera.h"
#include "NglProgram.h"
#include "NglTerrainGeometry.h"
#include "NglTexture.h"
#include "NglVertexArray.h"
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

void doMain(GLFWwindow* window) {
    NglTerrainGeometry terrainGeometry;

    Assimp::Importer importer;

    const char* sceneFileName = "simple-man.glb";
    const aiScene* scene =
            importer.ReadFile(sceneFileName, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs |
                                                     aiProcess_JoinIdenticalVertices);
    if (scene) {
        NGL_LOGI("%s loaded", sceneFileName);
    } else {
        NGL_LOGE("Error loading %s: %s", sceneFileName, importer.GetErrorString());
        abort();
    }

    std::vector<NglVertex> soldierVertices;
    std::vector<uint32_t> soldierIndices;
    for (unsigned int m = 0; m < scene->mNumMeshes; m++) {
        const aiMesh* mesh = scene->mMeshes[m];
        NGL_ASSERT(mesh->HasTextureCoords(0));
        for (unsigned int v = 0; v < mesh->mNumVertices; v++) {
            NglVertex vertex;
            vertex.position = ai2glm(mesh->mVertices[v]);
            vertex.position += glm::vec3(0, 10, 0);  // Move up
            vertex.normal = ai2glm(mesh->mNormals[v]);
            vertex.uv = ai2glmvec2(mesh->mTextureCoords[0][v]);
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

    // Terrain

    // VAO
    NglVertexArray terrainVao;
    glBindVertexArray(terrainVao);
    NGL_CHECK_ERRORS;

    double startTime = glfwGetTime();
    std::vector<NglVertex> terrainVertices;
    std::vector<uint32_t> terrainIndices;
    terrainGeometry.getData(&terrainVertices, &terrainIndices);
    double terrainGenerationTime = glfwGetTime() - startTime;
    NGL_LOGI("Terrain generation time: %0.3fs", terrainGenerationTime);

    // Vertices
    NglBuffer terrainVertexBuffer;
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

    glVertexArrayAttribFormat(terrainVao, 2 /*uv*/, 2, GL_FLOAT, GL_FALSE, 0);
    NGL_CHECK_ERRORS;
    glVertexArrayAttribBinding(terrainVao, 2, 2);
    NGL_CHECK_ERRORS;
    glVertexArrayVertexBuffer(terrainVao, 2, terrainVertexBuffer, offsetof(NglVertex, uv), sizeof(NglVertex));
    NGL_CHECK_ERRORS;
    glEnableVertexArrayAttrib(terrainVao, 2);
    NGL_CHECK_ERRORS;

    // Indices
    NglBuffer terrainIndexBuffer;
    glNamedBufferStorage(terrainIndexBuffer, terrainIndices.size() * sizeof(uint32_t), terrainIndices.data(), 0);
    NGL_CHECK_ERRORS;
    glVertexArrayElementBuffer(terrainVao, terrainIndexBuffer);
    NGL_CHECK_ERRORS;

    // Texture
    NglTexture terrainTexture("terrain-texture.png");

    glBindVertexArray(0);
    NGL_CHECK_ERRORS;

    // Soldier

    // VAO
    NglVertexArray soldierVao;
    glBindVertexArray(soldierVao);
    NGL_CHECK_ERRORS;

    // Vertices
    NglBuffer soldierVertexBuffer;
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

    glVertexArrayAttribFormat(soldierVao, 2 /*uv*/, 2, GL_FLOAT, GL_FALSE, 0);
    NGL_CHECK_ERRORS;
    glVertexArrayAttribBinding(soldierVao, 2, 2);
    NGL_CHECK_ERRORS;
    glVertexArrayVertexBuffer(soldierVao, 2, soldierVertexBuffer, offsetof(NglVertex, uv), sizeof(NglVertex));
    NGL_CHECK_ERRORS;
    glEnableVertexArrayAttrib(soldierVao, 2);
    NGL_CHECK_ERRORS;

    // Indices
    NglBuffer soldierIndexBuffer;
    glNamedBufferStorage(soldierIndexBuffer, soldierIndices.size() * sizeof(uint32_t), soldierIndices.data(), 0);
    NGL_CHECK_ERRORS;
    glVertexArrayElementBuffer(soldierVao, soldierIndexBuffer);
    NGL_CHECK_ERRORS;

    // Texture
    NGL_ASSERT(scene->mNumMaterials > 0);
    const aiMaterial* material = scene->mMaterials[0];
    NGL_LOGI("Material: %s", material->GetName().C_Str());
    NGL_ASSERT(material->GetName().length > 0);
    NGL_LOGI("Diffuse texture count: %u", material->GetTextureCount(aiTextureType_DIFFUSE));
    aiString path;
    NGL_ASSERT(material->GetTexture(aiTextureType_DIFFUSE, 0, &path) == AI_SUCCESS);
    NGL_LOGI("Diffuse texture 0, path: %s", path.C_Str());
    const aiTexture* aiTexture = scene->GetEmbeddedTexture(path.C_Str());
    NGL_ASSERT(aiTexture);
    NGL_LOGI("Diffuse texture 0, width: %u, height: %u", aiTexture->mWidth, aiTexture->mHeight);
    NGL_ASSERT(aiTexture->pcData);
    NGL_ASSERT(aiTexture->mHeight == 0);
    NGL_ASSERT(aiTexture->mWidth > 0);
    NglTexture soldierTexture(aiTexture->pcData, aiTexture->mWidth, "Diffuse texture 0");

    glBindVertexArray(0);
    NGL_CHECK_ERRORS;

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
        NGL_CHECK_ERRORS;
        terrainTexture.bind(0);
        glDrawElements(GL_TRIANGLES, static_cast<int>(std::size(terrainIndices)), GL_UNSIGNED_INT, 0);
        NGL_CHECK_ERRORS;

        glBindVertexArray(soldierVao);
        NGL_CHECK_ERRORS;
        soldierTexture.bind(0);
        glDrawElements(GL_TRIANGLES, static_cast<int>(std::size(soldierIndices)), GL_UNSIGNED_INT, 0);
        NGL_CHECK_ERRORS;

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

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

    doMain(window);

    glfwDestroyWindow(window);
    glfwTerminate();

    // TODO: Layers
    // TODO: Army of soldiers
    // TODO: Snapping to terrain
    // TODO: Movement
    // TODO: Nice rendering (lighting, grass material, cloth material?)
    // TODO: Animation
    // TODO: Vulkan
    return 0;
}
