#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <stb_image.h>
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

    glVertexArrayAttribFormat(terrainVao, 2 /*uv*/, 2, GL_FLOAT, GL_FALSE, 0);
    NGL_CHECK_ERRORS;
    glVertexArrayAttribBinding(terrainVao, 2, 2);
    NGL_CHECK_ERRORS;
    glVertexArrayVertexBuffer(terrainVao, 2, terrainVertexBuffer, offsetof(NglVertex, uv), sizeof(NglVertex));
    NGL_CHECK_ERRORS;
    glEnableVertexArrayAttrib(terrainVao, 2);
    NGL_CHECK_ERRORS;

    // Indices
    GLuint terrainIndexBuffer;
    glCreateBuffers(1, &terrainIndexBuffer);
    NGL_CHECK_ERRORS;
    glNamedBufferStorage(terrainIndexBuffer, terrainIndices.size() * sizeof(uint32_t), terrainIndices.data(), 0);
    NGL_CHECK_ERRORS;
    glVertexArrayElementBuffer(terrainVao, terrainIndexBuffer);
    NGL_CHECK_ERRORS;

    // Texture
    GLuint terrainTexture;
    {
        const char* filename = "terrain-texture.png";
        int width;
        int height;
        unsigned char* pixels = stbi_load(filename, &width, &height, nullptr, STBI_rgb);
        NGL_ASSERT(pixels);
        NGL_LOGI("%s loaded, width: %d, height: %d", filename, width, height);

        glCreateTextures(GL_TEXTURE_2D, 1, &terrainTexture);
        NGL_CHECK_ERRORS;
        glTextureParameteri(terrainTexture, GL_TEXTURE_MAX_LEVEL, 0);
        NGL_CHECK_ERRORS;
        glTextureParameteri(terrainTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        NGL_CHECK_ERRORS;
        glTextureParameteri(terrainTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        NGL_CHECK_ERRORS;
        glTextureStorage2D(terrainTexture, 1, GL_RGB8, width, height);
        NGL_CHECK_ERRORS;
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        NGL_CHECK_ERRORS;
        glTextureSubImage2D(terrainTexture, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels);
        NGL_CHECK_ERRORS;

        stbi_image_free(pixels);
    }

    glBindVertexArray(0);
    NGL_CHECK_ERRORS;

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

    glVertexArrayAttribFormat(soldierVao, 2 /*uv*/, 2, GL_FLOAT, GL_FALSE, 0);
    NGL_CHECK_ERRORS;
    glVertexArrayAttribBinding(soldierVao, 2, 2);
    NGL_CHECK_ERRORS;
    glVertexArrayVertexBuffer(soldierVao, 2, soldierVertexBuffer, offsetof(NglVertex, uv), sizeof(NglVertex));
    NGL_CHECK_ERRORS;
    glEnableVertexArrayAttrib(soldierVao, 2);
    NGL_CHECK_ERRORS;

    // Indices
    GLuint soldierIndexBuffer;
    glCreateBuffers(1, &soldierIndexBuffer);
    NGL_CHECK_ERRORS;
    glNamedBufferStorage(soldierIndexBuffer, soldierIndices.size() * sizeof(uint32_t), soldierIndices.data(), 0);
    NGL_CHECK_ERRORS;
    glVertexArrayElementBuffer(soldierVao, soldierIndexBuffer);
    NGL_CHECK_ERRORS;

    // Texture
    GLuint soldierTexture;
    {
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
        NGL_ASSERT(aiTexture->mHeight == 0);
        NGL_ASSERT(aiTexture->mWidth > 0);

        int width;
        int height;
        unsigned char* pixels = stbi_load_from_memory(reinterpret_cast<const stbi_uc*>(aiTexture->pcData),
                                                      aiTexture->mWidth, &width, &height, nullptr, STBI_rgb);
        NGL_ASSERT(pixels);
        NGL_LOGI("Diffuse texture 0 loaded, width: %d, height: %d", width, height);

        glCreateTextures(GL_TEXTURE_2D, 1, &soldierTexture);
        NGL_CHECK_ERRORS;
        glTextureParameteri(soldierTexture, GL_TEXTURE_MAX_LEVEL, 0);
        NGL_CHECK_ERRORS;
        glTextureParameteri(soldierTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        NGL_CHECK_ERRORS;
        glTextureParameteri(soldierTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        NGL_CHECK_ERRORS;
        glTextureStorage2D(soldierTexture, 1, GL_RGB8, width, height);
        NGL_CHECK_ERRORS;
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        NGL_CHECK_ERRORS;
        glTextureSubImage2D(soldierTexture, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels);
        NGL_CHECK_ERRORS;

        stbi_image_free(pixels);
    }

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
        glBindTextures(0, 1, &terrainTexture);
        NGL_CHECK_ERRORS;
        glDrawElements(GL_TRIANGLES, static_cast<int>(std::size(terrainIndices)), GL_UNSIGNED_INT, 0);
        NGL_CHECK_ERRORS;

        glBindVertexArray(soldierVao);
        NGL_CHECK_ERRORS;
        glBindTextures(0, 1, &soldierTexture);
        NGL_CHECK_ERRORS;
        glDrawElements(GL_TRIANGLES, static_cast<int>(std::size(soldierIndices)), GL_UNSIGNED_INT, 0);
        NGL_CHECK_ERRORS;

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteTextures(1, &soldierTexture);
    glDeleteBuffers(1, &soldierIndexBuffer);
    glDeleteBuffers(1, &soldierVertexBuffer);
    glDeleteVertexArrays(1, &soldierVao);

    glDeleteTextures(1, &terrainTexture);
    glDeleteBuffers(1, &terrainIndexBuffer);
    glDeleteBuffers(1, &terrainVertexBuffer);
    glDeleteVertexArrays(1, &terrainVao);

    glDeleteBuffers(1, &frameUniformBuffer);

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
