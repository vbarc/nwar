#pragma once

static const char* gFragmentShaderSrc = R"(
#version 460 core

in GS_OUT {
    vec2 uv;
    vec3 diffuse_factor;
    vec3 specular_component;
    vec3 barycoords;
} fs_in;

layout (location = 0) out vec4 out_color;

layout (std140, binding = 0) uniform FrameUniform {
    mat4 model_view_matrix;
    mat4 projection_matrix;
    int is_wireframe_enabled;
} frame;

layout (binding = 1) uniform sampler2D texture0;

const vec3 ambient_factor = vec3(0.4);

void main() {
    vec4 color = texture(texture0, fs_in.uv);
    color = color * vec4(fs_in.diffuse_factor + ambient_factor, 1) + vec4(fs_in.specular_component, 1);
    if (frame.is_wireframe_enabled != 0) {
        vec3 edge_factor = smoothstep(vec3(0.0), fwidth(fs_in.barycoords), fs_in.barycoords);
        float min_edge_factor = min(min(edge_factor.x, edge_factor.y), edge_factor.z);
        out_color = mix(vec4(color.rgb * 0.2, 1.0), color, min_edge_factor);
    } else {
        out_color = color;
    }
}
)";
