#pragma once

static const char* gFragmentShaderSrc = R"(
#version 460 core

layout (location=0) in vec4 gsout_color;
layout (location=1) in vec3 gsout_barycoords;
layout (location=0) out vec4 fsout_color;

layout (std140, binding = 0) uniform FrameUniform {
    mat4 model_view_matrix;
    mat4 projection_matrix;
    int is_wireframe_enabled;
};

void main() {
    if (is_wireframe_enabled != 0) {
        vec3 edge_factor = smoothstep(vec3(0.0), fwidth(gsout_barycoords), gsout_barycoords);
        float min_edge_factor = min(min(edge_factor.x, edge_factor.y), edge_factor.z);
        fsout_color = mix(vec4(gsout_color.rgb * 0.2, 1.0), gsout_color, min_edge_factor);
    } else {
        fsout_color = gsout_color;
    }
}
)";
