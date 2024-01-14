#pragma once

#include <glm/ext.hpp>
#include <glm/glm.hpp>

struct NglVertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv = glm::vec2(0, 0);
    int32_t type = 0;
};
