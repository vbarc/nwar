#include "NglTerrainLayer.h"

#include "nglerr.h"
#include "ngllog.h"

NglTerrainLayer::NglTerrainLayer(const NglTerrainGeometry& terrainGeometry) {
    const std::vector<NglVertex>& vertices = terrainGeometry.vertices();
    const std::vector<uint32_t>& indices = terrainGeometry.indices();

    // VAO
    glBindVertexArray(mVao);
    NGL_CHECK_ERRORS;

    // Vertex buffer
    glNamedBufferStorage(mVertexBuffer, vertices.size() * sizeof(NglVertex), vertices.data(), 0);
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
    mIndexCount = static_cast<GLsizei>(indices.size());
    glNamedBufferStorage(mIndexBuffer, mIndexCount * sizeof(uint32_t), indices.data(), 0);
    NGL_CHECK_ERRORS;
    glVertexArrayElementBuffer(mVao, mIndexBuffer);
    NGL_CHECK_ERRORS;

    // Texture
    mTexture.load("terrain-texture.png");

    glBindVertexArray(0);
    NGL_CHECK_ERRORS;
}

NglTerrainLayer::~NglTerrainLayer() {}

void NglTerrainLayer::draw() {
    glBindVertexArray(mVao);
    NGL_CHECK_ERRORS;
    mTexture.bind(0);
    glDrawElements(GL_TRIANGLES, mIndexCount, GL_UNSIGNED_INT, 0);
    NGL_CHECK_ERRORS;
}
