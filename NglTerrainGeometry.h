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

    const std::vector<NglVertex>& vertices() const;
    const std::vector<uint32_t>& indices() const;

private:
    std::vector<NglVertex> mVertices;
    std::vector<uint32_t> mIndices;
};
