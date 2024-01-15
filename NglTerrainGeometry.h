#pragma once

#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <vector>

#include "NglVertex.h"

class NglTerrainGeometry {
public:
    NglTerrainGeometry();
    NglTerrainGeometry(const NglTerrainGeometry&) = delete;
    NglTerrainGeometry& operator=(const NglTerrainGeometry&) = delete;
    NglTerrainGeometry(NglTerrainGeometry&&) = delete;
    NglTerrainGeometry& operator=(NglTerrainGeometry&&) = delete;
    ~NglTerrainGeometry();

    void getData(std::vector<NglVertex>* verticesOut, std::vector<uint32_t>* indicesOut);

private:
    float sample(int pixelX, int pixelZ);

    int mPixelWidth;
    int mPixelDepth;
    unsigned char* mPixelData;
};
