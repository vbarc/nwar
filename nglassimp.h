#pragma once

#include <assimp/types.h>

#include <glm/ext.hpp>
#include <glm/glm.hpp>

inline glm::vec3 ai2glm(const aiVector3D& aiVector) {
    return glm::vec3(aiVector.x, aiVector.y, aiVector.z);
}
