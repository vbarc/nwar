#pragma once

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

    int width() const;
    int depth() const;

    const std::vector<NglVertex>& vertices() const;
    const std::vector<uint32_t>& indices() const;
    const std::vector<float>& heights() const;

private:
    std::vector<NglVertex> mVertices;
    std::vector<uint32_t> mIndices;
    std::vector<float> mHeights;
};
