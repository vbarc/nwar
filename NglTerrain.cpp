#include "NglTerrain.h"

#include <algorithm>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "nglassert.h"
#include "ngllog.h"

using glm::vec3;

constexpr float kMinX = -100.0f;
constexpr float kMaxX = 100.0f;
constexpr float kMinZ = -100.0f;
constexpr float kMaxZ = 100.0f;
constexpr float kMinY = 0.0f;
constexpr float kMaxY = 1.0f;
constexpr int kGranularity = 100;

NglTerrain::NglTerrain() {
    int n;
    mPixelData = stbi_load("terrain.png", &mPixelWidth, &mPixelDepth, &n, 1);
    NGL_LOGI("Terrain loaded, data: %p, width: %d, depth: %d, n: %d", mPixelData, mPixelWidth, mPixelDepth, n);
    NGL_ASSERT(mPixelData);
    for (int r = 0; r < mPixelDepth; r++) {
        for (int c = 0; c < mPixelWidth; c++) {
            NGL_LOGI("(%02d, %02d): %d", r, c, mPixelData[r * mPixelWidth + c]);
        }
    }
}

NglTerrain::~NglTerrain() {
    stbi_image_free(mPixelData);
}

void NglTerrain::getData(std::vector<glm::vec3>* verticesOut, std::vector<uint32_t>* indicesOut) {
    verticesOut->reserve((kGranularity + 1) * (kGranularity + 1));
    for (int j = 0; j <= kGranularity; j++) {
        for (int i = 0; i <= kGranularity; i++) {
            float x = kMinX + (kMaxX - kMinX) / kGranularity * i;
            float z = kMinZ + (kMaxZ - kMinZ) / kGranularity * j;
            int c = std::min(static_cast<int>(mPixelWidth * 1.0f / kGranularity * i), mPixelWidth - 1);
            int r = std::min(static_cast<int>(mPixelDepth * 1.0f / kGranularity * j), mPixelDepth - 1);
            float y = kMinY + (kMaxY - kMinY) / 255 * mPixelData[r * mPixelWidth + c];
            verticesOut->push_back(vec3(x, y, z));
        }
    }

    indicesOut->reserve(kGranularity * kGranularity * 6);
    for (int j = 0; j < kGranularity; j++) {
        for (int i = 0; i < kGranularity; i++) {
            uint32_t baseIndex = j * (kGranularity + 1) + i;
            indicesOut->push_back(baseIndex);
            indicesOut->push_back(baseIndex + 1);
            indicesOut->push_back(baseIndex + kGranularity + 1);
            indicesOut->push_back(baseIndex + 1);
            indicesOut->push_back(baseIndex + kGranularity + 2);
            indicesOut->push_back(baseIndex + kGranularity + 1);
        }
    }
}
