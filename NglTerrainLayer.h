#pragma once

#include "NglBuffer.h"
#include "NglTexture.h"
#include "NglVertexArray.h"

class NglTerrainLayer {
public:
    NglTerrainLayer();
    NglTerrainLayer(const NglTerrainLayer&) = delete;
    NglTerrainLayer& operator=(const NglTerrainLayer&) = delete;
    NglTerrainLayer(NglTerrainLayer&&) = delete;
    NglTerrainLayer& operator=(NglTerrainLayer&&) = delete;
    ~NglTerrainLayer();

    void draw();

private:
    const NglVertexArray mVao;
    const NglBuffer mVertexBuffer;
    const NglBuffer mIndexBuffer;
    const NglTexture mTexture;
    GLsizei mIndexCount;
};
