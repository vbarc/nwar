#include "NglTerrain.h"

#include <algorithm>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "NglBicubicInterpolation.h"
#include "nglassert.h"
#include "ngllog.h"

using glm::vec3;

constexpr float kMinX = -100.0f;
constexpr float kMaxX = 100.0f;
constexpr float kMinZ = -100.0f;
constexpr float kMaxZ = 100.0f;
constexpr float kMinY = 0.0f;
constexpr float kMaxY = 5.0f;
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
            int basePixelX =
                    std::min(static_cast<int>((mPixelWidth - 3) * 1.0 / kGranularity * i) + 1, mPixelWidth - 3);
            int basePixelZ =
                    std::min(static_cast<int>((mPixelDepth - 3) * 1.0 / kGranularity * j) + 1, mPixelDepth - 3);
            float onePixelWidth = (kMaxX - kMinX) / (mPixelWidth - 3);
            float onePixelDepth = (kMaxZ - kMinZ) / (mPixelDepth - 3);
            float baseX = kMinX + onePixelWidth * (basePixelX - 1);
            float baseZ = kMinZ + onePixelDepth * (basePixelZ - 1);
            float normalizedX = (x - baseX) / onePixelWidth;
            float normalizedZ = (z - baseZ) / onePixelDepth;

            float f[16] = {
                    sample(basePixelX - 1, basePixelZ - 1),  //
                    sample(basePixelX + 0, basePixelZ - 1),  //
                    sample(basePixelX + 1, basePixelZ - 1),  //
                    sample(basePixelX + 2, basePixelZ - 1),  //
                    sample(basePixelX - 1, basePixelZ + 0),  //
                    sample(basePixelX + 0, basePixelZ + 0),  //
                    sample(basePixelX + 1, basePixelZ + 0),  //
                    sample(basePixelX + 2, basePixelZ + 0),  //
                    sample(basePixelX - 1, basePixelZ + 1),  //
                    sample(basePixelX + 0, basePixelZ + 1),  //
                    sample(basePixelX + 1, basePixelZ + 1),  //
                    sample(basePixelX + 2, basePixelZ + 1),  //
                    sample(basePixelX - 1, basePixelZ + 2),  //
                    sample(basePixelX + 0, basePixelZ + 2),  //
                    sample(basePixelX + 1, basePixelZ + 2),  //
                    sample(basePixelX + 2, basePixelZ + 2),  //
            };
            NglBicubicInterpolation interp(f);

            float y = interp.interpolate(normalizedX, normalizedZ);

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

float NglTerrain::sample(int pixelX, int pixelZ) {
    NGL_ASSERT(pixelX >= 0);
    NGL_ASSERT(pixelX < mPixelWidth);
    NGL_ASSERT(pixelZ >= 0);
    NGL_ASSERT(pixelZ < mPixelDepth);
    return kMinY + (kMaxY - kMinY) / 255 * mPixelData[pixelZ * mPixelWidth + pixelX];
}
