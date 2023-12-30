#include "NglTerrain.h"

#include <Eigen/Dense>
#include <algorithm>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "nglassert.h"
#include "ngllog.h"

using glm::vec3;

using EVector = Eigen::Vector<float, 16>;
using EMatrix = Eigen::Matrix<float, 16, 16>;

constexpr float kMinX = -100.0f;
constexpr float kMaxX = 100.0f;
constexpr float kMinZ = -100.0f;
constexpr float kMaxZ = 100.0f;
constexpr float kMinY = 0.0f;
constexpr float kMaxY = 5.0f;
constexpr int kGranularity = 100;
const auto kSolver =
        EMatrix{
                {1, -1, 1, -1, -1, 1, -1, 1, 1, -1, 1, -1, -1, 1, -1, 1},  //
                {1, 0, 0, 0, -1, 0, 0, 0, 1, 0, 0, 0, -1, 0, 0, 0},        //
                {1, 1, 1, 1, -1, -1, -1, -1, 1, 1, 1, 1, -1, -1, -1, -1},  //
                {1, 2, 4, 8, -1, -2, -4, -8, 1, 2, 4, 8, -1, -2, -4, -8},  //
                //
                {1, -1, 1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  //
                {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},    //
                {1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},    //
                {1, 2, 4, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},    //
                //
                {1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1, 1, -1},  //
                {1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0},          //
                {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},          //
                {1, 2, 4, 8, 1, 2, 4, 8, 1, 2, 4, 8, 1, 2, 4, 8},          //
                //
                {1, -1, 1, -1, 2, -2, 2, -2, 4, -4, 4, -4, 8, -8, 8, -8},  //
                {1, 0, 0, 0, 2, 0, 0, 0, 4, 0, 0, 0, 8, 0, 0, 0},          //
                {1, 1, 1, 1, 2, 2, 2, 2, 4, 4, 4, 4, 8, 8, 8, 8},          //
                {1, 2, 4, 8, 2, 4, 8, 16, 4, 8, 16, 32, 8, 16, 32, 64},    //
        }
                .colPivHouseholderQr();

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

static float interpolateBicubic(const EVector& f, float x, float y) {
    EVector a = kSolver.solve(f);
    return a(0) + a(1) * x + a(2) * x * x + a(3) * x * x * x                                      //
           + a(4) * y + a(5) * y * x + a(6) * y * x * x + a(7) * y * x * x * x                    //
           + a(8) * y * y + a(9) * y * y * x + a(10) * y * y * x * x + a(11) * y * y * x * x * x  //
           + a(12) * y * y * y + a(13) * y * y * y * x + a(14) * y * y * y * x * x + a(15) * y * y * y * x * x * x;
}

void NglTerrain::getData(std::vector<glm::vec3>* verticesOut, std::vector<uint32_t>* indicesOut) {
    verticesOut->reserve((kGranularity + 1) * (kGranularity + 1));
    for (int j = 0; j <= kGranularity; j++) {
        for (int i = 0; i <= kGranularity; i++) {
            float x = kMinX + (kMaxX - kMinX) / kGranularity * i;
            float z = kMinZ + (kMaxZ - kMinZ) / kGranularity * j;
            int basePixelX = std::min(static_cast<int>((mPixelWidth - 1) * 1.0 / kGranularity * i), mPixelWidth - 2);
            int basePixelZ = std::min(static_cast<int>((mPixelDepth - 1) * 1.0 / kGranularity * j), mPixelDepth - 2);
            float onePixelWidth = (kMaxX - kMinX) / (mPixelWidth - 1);
            float onePixelDepth = (kMaxZ - kMinZ) / (mPixelDepth - 1);
            float baseX = kMinX + onePixelWidth * basePixelX;
            float baseZ = kMinZ + onePixelDepth * basePixelZ;
            float normalizedX = (x - baseX) / onePixelWidth;
            float normalizedZ = (z - baseZ) / onePixelDepth;

            EVector f;
            f(5) = sample(basePixelX, basePixelZ);
            f(6) = sample(basePixelX + 1, basePixelZ);
            f(9) = sample(basePixelX, basePixelZ + 1);
            f(10) = sample(basePixelX + 1, basePixelZ + 1);

            if (basePixelX > 0) {
                f(4) = sample(basePixelX - 1, basePixelZ);
                f(8) = sample(basePixelX - 1, basePixelZ + 1);
            } else {
                f(4) = f(5) + f(5) - f(6);
                f(8) = f(9) + f(9) - f(10);
            }
            if (basePixelZ > 0) {
                f(1) = sample(basePixelX, basePixelZ - 1);
                f(2) = sample(basePixelX + 1, basePixelZ - 1);
            } else {
                f(1) = f(5) + f(5) - f(9);
                f(2) = f(6) + f(6) - f(10);
            }

            if (basePixelX < mPixelWidth - 2) {
                f(7) = sample(basePixelX + 2, basePixelZ);
                f(11) = sample(basePixelX + 2, basePixelZ + 1);
            } else {
                f(7) = f(6) + f(6) - f(5);
                f(11) = f(10) + f(10) - f(9);
            }

            if (basePixelZ < mPixelDepth - 2) {
                f(13) = sample(basePixelX, basePixelZ + 2);
                f(14) = sample(basePixelX + 1, basePixelZ + 2);
            } else {
                f(13) = f(9) + f(9) - f(5);
                f(14) = f(10) + f(10) - f(6);
            }

            if (basePixelX > 0 && basePixelZ > 0) {
                f(0) = sample(basePixelX - 1, basePixelZ - 1);
            } else {
                f(0) = (f(4) + f(4) - f(8) + f(1) + f(1) - f(2)) / 2;
            }

            if (basePixelX < mPixelWidth - 2 && basePixelZ > 0) {
                f(3) = sample(basePixelX + 2, basePixelZ - 1);
            } else {
                f(3) = (f(2) + f(2) - f(1) + f(7) + f(7) - f(11)) / 2;
            }

            if (basePixelX > 0 && basePixelZ < mPixelDepth - 2) {
                f(12) = sample(basePixelX - 1, basePixelZ + 2);
            } else {
                f(12) = (f(8) + f(8) - f(4) + f(13) + f(13) - f(14)) / 2;
            }

            if (basePixelX < mPixelWidth - 2 && basePixelZ < mPixelDepth - 2) {
                f(15) = sample(basePixelX + 2, basePixelZ + 2);
            } else {
                f(15) = (f(14) + f(14) - f(13) + f(11) + f(11) - f(7)) / 2;
            }

            float y = interpolateBicubic(f, normalizedX, normalizedZ);

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
