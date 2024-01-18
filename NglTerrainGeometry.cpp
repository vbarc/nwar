#include "NglTerrainGeometry.h"

#include <algorithm>

#include "NglBicubicInterpolation.h"
#include "NglDisplacementMap.h"
#include "nglassert.h"
#include "nglgl.h"
#include "ngllog.h"

using glm::vec2;
using glm::vec3;

constexpr float kMinX = -6.0f;
constexpr float kMaxX = 6.0f;
constexpr float kMinZ = -6.0f;
constexpr float kMaxZ = 6.0f;
constexpr float kMinY = 0.0f;
constexpr float kMaxY = 0.5f;
constexpr int kGranularity = 100;

static int index(int i, int j);
static vec3 vertexNormal(const std::vector<NglVertex>& vertices, int index1, int index2, int index3);

NglTerrainGeometry::NglTerrainGeometry() {
    double startTime = glfwGetTime();

    NglDisplacementMap dm("terrain-map.png");

    std::vector<std::vector<NglBicubicInterpolation>> interpolations;
    interpolations.resize(dm.depth() - 2);
    for (int j = 1; j < dm.depth() - 2; j++) {
        interpolations[j].resize(dm.width() - 2);
        for (int i = 1; i < dm.width() - 2; i++) {
            float f[16] = {
                    dm.lookup(i - 1, j - 1), dm.lookup(i + 0, j - 1), dm.lookup(i + 1, j - 1), dm.lookup(i + 2, j - 1),
                    dm.lookup(i - 1, j + 0), dm.lookup(i + 0, j + 0), dm.lookup(i + 1, j + 0), dm.lookup(i + 2, j + 0),
                    dm.lookup(i - 1, j + 1), dm.lookup(i + 0, j + 1), dm.lookup(i + 1, j + 1), dm.lookup(i + 2, j + 1),
                    dm.lookup(i - 1, j + 2), dm.lookup(i + 0, j + 2), dm.lookup(i + 1, j + 2), dm.lookup(i + 2, j + 2),
            };
            interpolations[j][i] = NglBicubicInterpolation(i, j, f);
        }
    }

    mVertices.reserve((kGranularity + 1) * (kGranularity + 1));
    for (int j = 0; j <= kGranularity; j++) {
        for (int i = 0; i <= kGranularity; i++) {
            float x = kMinX + (kMaxX - kMinX) / kGranularity * i;
            float z = kMinZ + (kMaxZ - kMinZ) / kGranularity * j;
            int basePixelX = std::min(static_cast<int>((dm.width() - 3) * 1.0 / kGranularity * i) + 1, dm.width() - 3);
            int basePixelZ = std::min(static_cast<int>((dm.depth() - 3) * 1.0 / kGranularity * j) + 1, dm.depth() - 3);
            float onePixelWidth = (kMaxX - kMinX) / (dm.width() - 3);
            float onePixelDepth = (kMaxZ - kMinZ) / (dm.depth() - 3);
            float baseX = kMinX + onePixelWidth * (basePixelX - 1);
            float baseZ = kMinZ + onePixelDepth * (basePixelZ - 1);
            float normalizedX = (x - baseX) / onePixelWidth;
            float normalizedZ = (z - baseZ) / onePixelDepth;

            float y = interpolations[basePixelZ][basePixelX].interpolate(normalizedX, normalizedZ);
            y = kMinY + (kMaxY - kMinY) * y;

            NglVertex vertex;
            vertex.position = vec3(x, y, z);
            vertex.uv = vec2(0);
            mVertices.push_back(vertex);
        }
    }

    for (int j = 0; j <= kGranularity; j++) {
        for (int i = 0; i <= kGranularity; i++) {
            vec3 normal = vec3(0);
            if (i > 0 && j > 0) {
                normal += vertexNormal(mVertices, index(i, j), index(i - 1, j), index(i, j - 1));
            }
            if (i < kGranularity && j > 0) {
                normal += vertexNormal(mVertices, index(i, j), index(i, j - 1), index(i + 1, j - 1));
                normal += vertexNormal(mVertices, index(i, j), index(i + 1, j - 1), index(i + 1, j));
            }
            if (i < kGranularity && j < kGranularity) {
                normal += vertexNormal(mVertices, index(i, j), index(i + 1, j), index(i, j + 1));
            }
            if (i > 0 && j < kGranularity) {
                normal += vertexNormal(mVertices, index(i, j), index(i, j + 1), index(i - 1, j + 1));
                normal += vertexNormal(mVertices, index(i, j), index(i - 1, j + 1), index(i - 1, j));
            }
            mVertices[index(i, j)].normal = glm::normalize(normal);
        }
    }

    mIndices.reserve(kGranularity * kGranularity * 6);
    for (int j = 0; j < kGranularity; j++) {
        for (int i = 0; i < kGranularity; i++) {
            mIndices.push_back(index(i, j));
            mIndices.push_back(index(i + 1, j));
            mIndices.push_back(index(i, j + 1));
            mIndices.push_back(index(i + 1, j));
            mIndices.push_back(index(i + 1, j + 1));
            mIndices.push_back(index(i, j + 1));
        }
    }

    NGL_LOGI("Terrain geometry generation time: %0.3fs", glfwGetTime() - startTime);
}

NglTerrainGeometry::~NglTerrainGeometry() {}

const std::vector<NglVertex>& NglTerrainGeometry::vertices() const {
    return mVertices;
}

const std::vector<uint32_t>& NglTerrainGeometry::indices() const {
    return mIndices;
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
