#pragma once

#include <vector>

#include "NwVertex.h"

class NwTerrainGeometry {
public:
    NwTerrainGeometry();
    NwTerrainGeometry(const NwTerrainGeometry&) = delete;
    NwTerrainGeometry& operator=(const NwTerrainGeometry&) = delete;
    NwTerrainGeometry(NwTerrainGeometry&&) = delete;
    NwTerrainGeometry& operator=(NwTerrainGeometry&&) = delete;
    ~NwTerrainGeometry();

    int width() const;
    int depth() const;

    const std::vector<NwVertex>& vertices() const;
    const std::vector<uint32_t>& indices() const;
    const std::vector<float>& heights() const;

private:
    std::vector<NwVertex> mVertices;
    std::vector<uint32_t> mIndices;
    std::vector<float> mHeights;
};
