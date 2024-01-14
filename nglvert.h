#pragma once

static const char* gVertexShaderSrc = R"(
#version 460 core

layout (location=0) in vec3 vsin_position;
layout (location=1) in vec3 vsin_normal;
layout (location=2) in vec2 vsin_uv;
layout (location=3) in int vsin_type;
layout (location=0) out vec4 vsout_color;
layout (location=1) out vec2 vsout_uv;
layout (location=2) out int vsout_type;
layout (location=3) out vec4 vsout_specular_color;
layout (location=4) out float vsout_diffuse_factor;

layout (std140, binding = 0) uniform FrameUniform {
    mat4 model_view_matrix;
    mat4 projection_matrix;
    int is_wireframe_enabled;
};

const vec3 light_position = vec3(-1100, 1200, 1000);
const vec3 ambient = vec3(0.1, 0.1, 0.1);
const vec3 diffuse_albedo = vec3(0, 0.7, 0);
const vec3 specular_albedo = vec3(0.1);
const float specular_power = 48;

void main() {
    vec3 light_vector = normalize(light_position - vsin_position);

    vec4 position_in_view = model_view_matrix * vec4(vsin_position, 1);
    vec3 normal_in_view = mat3(model_view_matrix) * vsin_normal;
    vec3 light_vector_in_view = mat3(model_view_matrix) * light_vector;
    vec3 view_vector_in_view = -position_in_view.xyz;

    normal_in_view = normalize(normal_in_view);
    light_vector_in_view = normalize(light_vector_in_view);
    view_vector_in_view = normalize(view_vector_in_view);

    vec3 reflection_vector_in_view = reflect(-light_vector_in_view, normal_in_view);

    float diffuse_factor = max(dot(normal_in_view, light_vector_in_view), 0);
    vec3 diffuse = diffuse_factor * diffuse_albedo;
    vec3 specular = pow(max(dot(reflection_vector_in_view, view_vector_in_view), 0), specular_power) * specular_albedo;

    vsout_color = vec4(ambient + diffuse + specular, 1);
    vsout_uv = vsin_uv;
    vsout_type = vsin_type;
    vsout_specular_color = vec4(specular, 1);
    vsout_diffuse_factor = diffuse_factor;

    gl_Position = projection_matrix * position_in_view;
}
)";
