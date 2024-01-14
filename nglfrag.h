#pragma once

static const char* gFragmentShaderSrc = R"(
#version 460 core

layout (location=0) in vec4 gsout_color;
layout (location=1) in vec2 gsout_uv;
layout (location=2) flat in int gsout_type;
layout (location=3) in vec3 gsout_diffuse_factor;
layout (location=4) in vec3 gsout_specular_component;
layout (location=5) in vec3 gsout_barycoords;
layout (location=0) out vec4 fsout_color;

layout (std140, binding = 0) uniform FrameUniform {
    mat4 model_view_matrix;
    mat4 projection_matrix;
    int is_wireframe_enabled;
};

uniform sampler2D texture0;

const vec3 ambient_factor = vec3(0.4);

void main() {
    vec4 color;
    if (gsout_type == 0) {
        color = gsout_color;
    } else {
        color = texture(texture0, gsout_uv);
        color = color * vec4(gsout_diffuse_factor + ambient_factor, 1) + vec4(gsout_specular_component, 1);
    }
    if (is_wireframe_enabled != 0) {
        vec3 edge_factor = smoothstep(vec3(0.0), fwidth(gsout_barycoords), gsout_barycoords);
        float min_edge_factor = min(min(edge_factor.x, edge_factor.y), edge_factor.z);
        fsout_color = mix(vec4(color.rgb * 0.2, 1.0), color, min_edge_factor);
    } else {
        fsout_color = color;
    }
}
)";
