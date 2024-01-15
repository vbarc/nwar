#pragma once

#include "NglBuffer.h"
#include "NglTexture.h"
#include "NglVertexArray.h"

class NglArmyLayer {
public:
    NglArmyLayer();
    NglArmyLayer(const NglArmyLayer&) = delete;
    NglArmyLayer& operator=(const NglArmyLayer&) = delete;
    NglArmyLayer(NglArmyLayer&&) = delete;
    NglArmyLayer& operator=(NglArmyLayer&&) = delete;
    ~NglArmyLayer();

    void render();

private:
    const NglVertexArray mVao;
    const NglBuffer mVertexBuffer;
    const NglBuffer mIndexBuffer;
    const NglTexture mTexture;
    GLsizei mIndexCount;
};
