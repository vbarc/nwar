#include "NglArmyLayer.h"

#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <assimp/Importer.hpp>

#include "NglVertex.h"
#include "nglassert.h"
#include "nglassimp.h"
#include "nglerr.h"
#include "nglgl.h"
#include "ngllog.h"

constexpr glm::ivec2 kUnitSize = glm::ivec2(12, 12);
constexpr glm::ivec2 kUnitCount = glm::ivec2(3, 5);
constexpr int kRegimentCount = 4;
constexpr GLsizei kInstanceCount = kUnitSize.x * kUnitSize.y * kUnitCount.x * kUnitCount.y * kRegimentCount;
constexpr float kModelScale = 0.01f;

NglArmyLayer::NglArmyLayer(const NglTerrainGeometry&) {
    // GLTF model
    const char* path = "soldier.glb";
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals |
                                                           aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices);
    if (scene) {
        NGL_LOGI("%s loaded", path);
    } else {
        NGL_LOGE("Error loading %s: %s", path, importer.GetErrorString());
        abort();
    }

    std::vector<NglVertex> soldierVertices;
    std::vector<uint32_t> soldierIndices;
    float bottom = 0;
    for (unsigned int m = 0; m < scene->mNumMeshes; m++) {
        const aiMesh* mesh = scene->mMeshes[m];
        NGL_ASSERT(mesh->HasTextureCoords(0));
        for (unsigned int v = 0; v < mesh->mNumVertices; v++) {
            NglVertex vertex;
            vertex.position = ai2glm(mesh->mVertices[v]) * kModelScale;
            vertex.normal = ai2glm(mesh->mNormals[v]);
            vertex.uv = ai2glmvec2(mesh->mTextureCoords[0][v]);
            soldierVertices.push_back(vertex);
            bottom = std::min(bottom, vertex.position.y);
        }
        for (unsigned int f = 0; f < mesh->mNumFaces; f++) {
            const aiFace& face = mesh->mFaces[f];
            NGL_ASSERT(face.mNumIndices == 3);
            soldierIndices.push_back(face.mIndices[0]);
            soldierIndices.push_back(face.mIndices[1]);
            soldierIndices.push_back(face.mIndices[2]);
        }
    }

    // Rebase to y = 0
    for (NglVertex& vertex : soldierVertices) {
        vertex.position.y -= bottom;
        vertex.position.y += 0.5f;
    }

    // VAO
    glBindVertexArray(mVao);
    NGL_CHECK_ERRORS;

    // Vertex buffer
    glNamedBufferStorage(mVertexBuffer, soldierVertices.size() * sizeof(NglVertex), soldierVertices.data(), 0);
    NGL_CHECK_ERRORS;

    glVertexArrayAttribFormat(mVao, 0 /*position*/, 3, GL_FLOAT, GL_FALSE, 0);
    NGL_CHECK_ERRORS;
    glVertexArrayAttribBinding(mVao, 0, 0);
    NGL_CHECK_ERRORS;
    glVertexArrayVertexBuffer(mVao, 0, mVertexBuffer, 0, sizeof(NglVertex));
    NGL_CHECK_ERRORS;
    glEnableVertexArrayAttrib(mVao, 0);
    NGL_CHECK_ERRORS;

    glVertexArrayAttribFormat(mVao, 1 /*normal*/, 3, GL_FLOAT, GL_FALSE, 0);
    NGL_CHECK_ERRORS;
    glVertexArrayAttribBinding(mVao, 1, 1);
    NGL_CHECK_ERRORS;
    glVertexArrayVertexBuffer(mVao, 1, mVertexBuffer, offsetof(NglVertex, normal), sizeof(NglVertex));
    NGL_CHECK_ERRORS;
    glEnableVertexArrayAttrib(mVao, 1);
    NGL_CHECK_ERRORS;

    glVertexArrayAttribFormat(mVao, 2 /*uv*/, 2, GL_FLOAT, GL_FALSE, 0);
    NGL_CHECK_ERRORS;
    glVertexArrayAttribBinding(mVao, 2, 2);
    NGL_CHECK_ERRORS;
    glVertexArrayVertexBuffer(mVao, 2, mVertexBuffer, offsetof(NglVertex, uv), sizeof(NglVertex));
    NGL_CHECK_ERRORS;
    glEnableVertexArrayAttrib(mVao, 2);
    NGL_CHECK_ERRORS;

    // Index buffer
    mIndexCount = static_cast<GLsizei>(soldierIndices.size());
    glNamedBufferStorage(mIndexBuffer, mIndexCount * sizeof(uint32_t), soldierIndices.data(), 0);
    NGL_CHECK_ERRORS;
    glVertexArrayElementBuffer(mVao, mIndexBuffer);
    NGL_CHECK_ERRORS;

    // Texture
    NGL_ASSERT(scene->mNumMaterials > 0);
    const aiMaterial* material = scene->mMaterials[0];
    NGL_LOGI("Soldier material: %s", material->GetName().C_Str());
    NGL_ASSERT(material->GetName().length > 0);
    NGL_LOGI("Soldier diffuse texture count: %u", material->GetTextureCount(aiTextureType_DIFFUSE));
    aiString texturePath;
    NGL_ASSERT(material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == AI_SUCCESS);
    NGL_LOGI("Soldier diffuse texture 0, path: %s", texturePath.C_Str());
    const aiTexture* aiTexture = scene->GetEmbeddedTexture(texturePath.C_Str());
    NGL_ASSERT(aiTexture);
    NGL_LOGI("Soldier diffuse texture 0, width: %u, height: %u", aiTexture->mWidth, aiTexture->mHeight);
    NGL_ASSERT(aiTexture->pcData);
    NGL_ASSERT(aiTexture->mHeight == 0);
    NGL_ASSERT(aiTexture->mWidth > 0);
    mTexture.load(aiTexture->pcData, aiTexture->mWidth, "Soldier diffuse texture 0");

    glBindVertexArray(0);
    NGL_CHECK_ERRORS;
}

NglArmyLayer::~NglArmyLayer() {}

void NglArmyLayer::draw() {
    glBindVertexArray(mVao);
    NGL_CHECK_ERRORS;
    mTexture.bind(0);
    glDrawElementsInstancedBaseInstance(GL_TRIANGLES, mIndexCount, GL_UNSIGNED_INT, 0, kInstanceCount, 1);
    NGL_CHECK_ERRORS;
}
