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

static int index(int i, int j);
static vec3 vertexNormal(const std::vector<NglVertex>& vertices, int index1, int index2, int index3);

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

void NglTerrain::getData(std::vector<NglVertex>* verticesOut, std::vector<uint32_t>* indicesOut) {
    std::vector<std::vector<NglBicubicInterpolation>> interpolations;
    interpolations.resize(mPixelDepth - 2);
    for (int j = 1; j < mPixelDepth - 2; j++) {
        interpolations[j].resize(mPixelWidth - 2);
        for (int i = 1; i < mPixelWidth - 2; i++) {
            float f[16] = {
                    sample(i - 1, j - 1), sample(i + 0, j - 1), sample(i + 1, j - 1), sample(i + 2, j - 1),
                    sample(i - 1, j + 0), sample(i + 0, j + 0), sample(i + 1, j + 0), sample(i + 2, j + 0),
                    sample(i - 1, j + 1), sample(i + 0, j + 1), sample(i + 1, j + 1), sample(i + 2, j + 1),
                    sample(i - 1, j + 2), sample(i + 0, j + 2), sample(i + 1, j + 2), sample(i + 2, j + 2),
            };
            interpolations[j][i] = NglBicubicInterpolation(i, j, f);
        }
    }

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

            float y = interpolations[basePixelZ][basePixelX].interpolate(normalizedX, normalizedZ);

            NglVertex vertex;
            vertex.position = vec3(x, y, z);
            verticesOut->push_back(vertex);
        }
    }

    for (int j = 0; j <= kGranularity; j++) {
        for (int i = 0; i <= kGranularity; i++) {
            vec3 normal = vec3(0);
            if (i > 0 && j > 0) {
                normal += vertexNormal(*verticesOut, index(i, j), index(i - 1, j), index(i, j - 1));
            }
            if (i < kGranularity && j > 0) {
                normal += vertexNormal(*verticesOut, index(i, j), index(i, j - 1), index(i + 1, j - 1));
                normal += vertexNormal(*verticesOut, index(i, j), index(i + 1, j - 1), index(i + 1, j));
            }
            if (i < kGranularity && j < kGranularity) {
                normal += vertexNormal(*verticesOut, index(i, j), index(i + 1, j), index(i, j + 1));
            }
            if (i > 0 && j < kGranularity) {
                normal += vertexNormal(*verticesOut, index(i, j), index(i, j + 1), index(i - 1, j + 1));
                normal += vertexNormal(*verticesOut, index(i, j), index(i - 1, j + 1), index(i - 1, j));
            }
            (*verticesOut)[index(i, j)].normal = glm::normalize(normal);
        }
    }

    indicesOut->reserve(kGranularity * kGranularity * 6);
    for (int j = 0; j < kGranularity; j++) {
        for (int i = 0; i < kGranularity; i++) {
            indicesOut->push_back(index(i, j));
            indicesOut->push_back(index(i + 1, j));
            indicesOut->push_back(index(i, j + 1));
            indicesOut->push_back(index(i + 1, j));
            indicesOut->push_back(index(i + 1, j + 1));
            indicesOut->push_back(index(i, j + 1));
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

int index(int i, int j) {
    return j * (kGranularity + 1) + i;
}

vec3 vertexNormal(const std::vector<NglVertex>& vertices, int index1, int index2, int index3) {
    vec3 vertex1 = vertices[index1].position;
    vec3 vertex2 = vertices[index2].position;
    vec3 vertex3 = vertices[index3].position;
    return glm::normalize(glm::cross(vertex3 - vertex1, vertex2 - vertex1));
}
