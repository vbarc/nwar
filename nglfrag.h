#pragma once

static const char* gFragmentShaderSrc = R"(
#version 460 core

layout (location=0) in vec3 barypos;
layout (location=0) out vec4 color;

void main() {
    vec4 main_color = vec4(1, 0, 0, 1);
    vec3 edge_factor = smoothstep(vec3(0.0), fwidth(barypos), barypos);
    float min_edge_factor = min(min(edge_factor.x, edge_factor.y), edge_factor.z);
    color = mix(main_color * 0.2, main_color, min_edge_factor);
}
)";