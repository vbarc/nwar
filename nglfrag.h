#pragma once

static const char* gFragmentShaderSrc = R"(
#version 460 core

in GS_OUT {
    vec2 uv;
    vec3 color_factor;
    vec3 color_offset;
    vec3 barycoords;
} fs_in;

layout (location = 0) out vec4 out_color;

layout (std140, binding = 0) uniform FrameUniform {
    mat4 model_view_matrix;
    mat4 projection_matrix;
    float time;
    int is_wireframe_enabled;
} frame;

layout (binding = 1) uniform sampler2D colorTexture;

void main() {
    vec4 color = texture(colorTexture, fs_in.uv);
    color = color * vec4(fs_in.color_factor, 1) + vec4(fs_in.color_offset, 1);
    if (frame.is_wireframe_enabled != 0) {
        vec3 edge_factor = smoothstep(vec3(0.0), fwidth(fs_in.barycoords), fs_in.barycoords);
        float min_edge_factor = min(min(edge_factor.x, edge_factor.y), edge_factor.z);
        out_color = mix(vec4(color.rgb * 0.2, 1.0), color, min_edge_factor);
    } else {
        out_color = color;
    }
}
)";
