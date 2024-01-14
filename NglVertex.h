#pragma once

#include <glm/ext.hpp>
#include <glm/glm.hpp>

struct NglVertex {
    glm::vec3 position;
    glm::vec3 normal;
    int32_t type = 0;
};
