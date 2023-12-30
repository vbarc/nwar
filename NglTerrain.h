#pragma once

#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <vector>

class NglTerrain {
public:
    NglTerrain();
    ~NglTerrain();

    void getData(std::vector<glm::vec3>* verticesOut, std::vector<uint32_t>* indicesOut);

private:
    int mPixelWidth;
    int mPixelDepth;
    unsigned char* mPixelData;
};
