#pragma once

#include "NglBuffer.h"
#include "NglTerrainGeometry.h"
#include "NglTexture.h"
#include "NglVertexArray.h"

class NglArmyLayer {
public:
    NglArmyLayer(const NglTerrainGeometry& terrainGeometry);
    NglArmyLayer(const NglArmyLayer&) = delete;
    NglArmyLayer& operator=(const NglArmyLayer&) = delete;
    NglArmyLayer(NglArmyLayer&&) = delete;
    NglArmyLayer& operator=(NglArmyLayer&&) = delete;
    ~NglArmyLayer();

    void draw();

private:
    const NglVertexArray mVao;
    const NglBuffer mVertexBuffer;
    const NglBuffer mIndexBuffer;
    const NglTexture mTexture;
    GLsizei mIndexCount;
};
