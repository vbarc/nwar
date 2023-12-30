#pragma once

static const char* gFragmentShaderSrc = R"(
#version 460 core

layout (location=0) in vec3 barycoords;
layout (location=0) out vec4 color;

void main() {
    vec4 main_color = vec4(0, 0.6, 0, 1);
    vec3 edge_factor = smoothstep(vec3(0.0), fwidth(barycoords), barycoords);
    float min_edge_factor = min(min(edge_factor.x, edge_factor.y), edge_factor.z);
    color = mix(main_color * 0.2, main_color, min_edge_factor);
}
)";
