#pragma once

static const char* gVertexShaderSrc = R"(
#version 460 core

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec2 in_uv;

out VS_OUT {
    vec2 uv;
    vec3 diffuse_factor;
    vec3 specular_component;
} vs_out;

layout (std140, binding = 0) uniform FrameUniform {
    mat4 model_view_matrix;
    mat4 projection_matrix;
    int is_wireframe_enabled;
};

const ivec2 unit_size = ivec2(12, 12);
const ivec2 regiment_size = ivec2(3, 5);
const int regiment_count = 4;
const int unit_instance_count = unit_size.x * unit_size.y;
const int regiment_unit_count = regiment_size.x * regiment_size.y;
const int regiment_instance_count = regiment_unit_count * unit_instance_count;

const vec2 in_unit_distance = vec2(0.05, 0.05);
const vec2 unit_padding = vec2(0.12, 0.12);
const vec2 unit_distance = unit_size * in_unit_distance + unit_padding;;
const float regiment_padding = 0.2;
const float regiment_distance = regiment_size.y * unit_distance.y + regiment_padding;
const vec2 regiment_psize = regiment_size * unit_distance - unit_padding;

const vec3 light_position = vec3(-1100, 1200, 1000);
const vec3 specular_k = vec3(0.1);
const float specular_power = 48;

void main() {
    vec3 position;
    if (gl_BaseInstance == 0) {
        position = in_position;
    } else {
        int index = gl_InstanceID;

        int regiment_index = index / regiment_instance_count;
        index = index % regiment_instance_count;

        int unit_index = index / unit_instance_count;
        index = index % unit_instance_count;

        int unit_i = unit_index % regiment_size.x;
        int unit_j = unit_index / regiment_size.x;

        int in_unit_i = index % unit_size.x;
        int in_unit_j = index / unit_size.x;

        float x = unit_i * unit_distance.x + in_unit_i * in_unit_distance.x;
        float y = unit_j * unit_distance.y + in_unit_j * in_unit_distance.y + regiment_index * regiment_distance;
        x = x - regiment_psize.x / 2;
        y = -y + regiment_distance * regiment_count / 2;

        position = in_position + vec3(x, 0, y);
    }

    vec3 light_vector = normalize(light_position - position);

    vec4 position_in_view = model_view_matrix * vec4(position, 1);
    vec3 normal_in_view = mat3(model_view_matrix) * in_normal;
    vec3 light_vector_in_view = mat3(model_view_matrix) * light_vector;
    vec3 view_vector_in_view = -position_in_view.xyz;

    normal_in_view = normalize(normal_in_view);
    light_vector_in_view = normalize(light_vector_in_view);
    view_vector_in_view = normalize(view_vector_in_view);

    vec3 reflection_vector_in_view = reflect(-light_vector_in_view, normal_in_view);

    vec3 diffuse_factor = vec3(max(dot(normal_in_view, light_vector_in_view), 0));
    vec3 specular = pow(max(dot(reflection_vector_in_view, view_vector_in_view), 0), specular_power) * specular_k;

    vs_out.uv = in_uv;
    vs_out.diffuse_factor = diffuse_factor;
    vs_out.specular_component = specular;

    gl_Position = projection_matrix * position_in_view;
}
)";
