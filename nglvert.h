#pragma once

static const char* gVertexShaderSrc = R"(
#version 460 core

layout (std140, binding = 0) uniform FrameUniform {
    mat4 model_view_matrix;
    mat4 projection_matrix;
    float time;
    int is_wireframe_enabled;
} frame;

layout (binding = 0) uniform sampler2D terrain_texture;

layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec2 in_uv;

out VS_OUT {
    vec2 uv;
    vec3 color_factor;
    vec3 color_offset;
} vs_out;

const float pi = 3.14159265;
const float pi_x2 = 2 * pi;

const vec2 terrain_min = vec2(-6);
const vec2 terrain_max = vec2(6);
const vec2 path[] = {
    vec2(6, -3),
    vec2(3, -4),
    vec2(-2, 2),
    vec2(-6, 3),
};
const float len = length(path[3] - path[0]) * 1.25;

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
const float army_length = regiment_distance * regiment_count;

float speed = regiment_distance / 90;
float t_speed = speed / len;
float period = (1 + army_length / len) / t_speed;
float time0 = 0.2 * period;

float two_step_period = 0.5 * 2;
float phase_period = period / 50;

const vec3 light_position = vec3(-1100, 1200, 1000);
const vec3 ambient_factor = vec3(0.4);
const vec3 specular_factor = vec3(0.1);
const float specular_power = 48;

vec2 interpolate_along_path(float t, out vec2 dxy) {
    float t2 = t * t;
    float t3 = t2 * t;
    float it = 1 - t;
    float it2 = it * it;
    float it3 = it2 * it;
    vec2 result = it3 * path[0] + 3 * it2 * t * path[1] + 3 * it * t2 * path[2] + t3 * path[3];
    dxy = 3 * it2 * (path[1] - path[0]) + 6 * it * t * (path[2] - path[1]) + 3 * t2 * (path[3] - path[2]);
    return result;
}

float interpolate_terrain_height(float x, float z) {
    if (x < terrain_min.x || x > terrain_max.x || z < terrain_min.y || z > terrain_max.y) {
        return 0;
    }
    vec2 xz = (vec2(x, z) - terrain_min) / (terrain_max - terrain_min);
    return texture(terrain_texture, xz).r;
}

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


        float t_offset = regiment_index * regiment_distance + unit_j * unit_distance.y + in_unit_j * in_unit_distance.y;
        float effective_time = time0 + frame.time;
        float t = -t_offset / len + t_speed * mod(effective_time, period);

        vec2 dxz;
        vec2 xz = interpolate_along_path(t, dxz);

        vec2 path_dir = normalize(dxz);
        vec2 ort_dir = vec2(path_dir.y, -path_dir.x);

        float ort_offset = unit_i * unit_distance.x + in_unit_i * in_unit_distance.x - regiment_psize.x / 2;
        xz = xz + ort_dir * ort_offset;

        uint random_number = gl_InstanceID * 1103515245 + 12345;

        float phase = fract(effective_time / phase_period);
        vec2 random_phase = vec2(random_number & 1023, ((random_number >> 10) & 1023)) / 1024.0;
        vec2 effective_phase = (vec2(phase) + random_phase) * pi_x2;

        vec2 random_xz_offset = sin(effective_phase) / 300;
        xz = xz + random_xz_offset;

        float y = interpolate_terrain_height(xz.x, xz.y);

        float random_step_phase = ((random_number >> 5) & 1023) / 1024.0;
        float random_step_offset = sin(random_step_phase * pi_x2) * 0.25;
        float two_step_t = fract(effective_time / two_step_period + random_step_offset);
        float step_t = fract(two_step_t * 2);
        float step_y = sin(step_t * pi) * 0.003;
        y = y + step_y;

        mat3 path_orientation = mat3(
            path_dir.y, 0, -path_dir.x,
            0, 1, 0,
            path_dir.x, 0, path_dir.y);

        float theta = sin(two_step_t * pi_x2) * 0.03;
        float sin_theta = sin(theta);
        float cos_theta = cos(theta);
        mat3 swing_orientation = mat3(
            cos_theta, sin_theta, 0,
            -sin_theta, cos_theta, 0,
            0, 0, 1
        );

        float y_scale = (random_number & 63) / 448.0;
        vec3 scale = vec3(1, 1 + y_scale, 1);

        position = path_orientation * swing_orientation * (scale * in_position) + vec3(xz.x, y, xz.y);
    }

    vec3 light_vector = normalize(light_position - position);

    vec4 position_in_view = frame.model_view_matrix * vec4(position, 1);
    vec3 normal_in_view = mat3(frame.model_view_matrix) * in_normal;
    vec3 light_vector_in_view = mat3(frame.model_view_matrix) * light_vector;
    vec3 view_vector_in_view = -position_in_view.xyz;

    normal_in_view = normalize(normal_in_view);
    light_vector_in_view = normalize(light_vector_in_view);
    view_vector_in_view = normalize(view_vector_in_view);

    vec3 reflection_vector_in_view = reflect(-light_vector_in_view, normal_in_view);

    vec3 diffuse_factor = vec3(max(dot(normal_in_view, light_vector_in_view), 0));
    vec3 specular = pow(max(dot(reflection_vector_in_view, view_vector_in_view), 0), specular_power) * specular_factor;

    vs_out.uv = in_uv;
    vs_out.color_factor = diffuse_factor + ambient_factor;
    vs_out.color_offset = specular;

    gl_Position = frame.projection_matrix * position_in_view;
}
)";
