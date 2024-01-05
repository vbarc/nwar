#pragma once

#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <vector>

#include "NglVertex.h"

class NglTerrain {
public:
    NglTerrain();
    ~NglTerrain();

    void getData(std::vector<NglVertex>* verticesOut, std::vector<uint32_t>* indicesOut);

private:
    float sample(int pixelX, int pixelZ);

    int mPixelWidth;
    int mPixelDepth;
    unsigned char* mPixelData;
};
