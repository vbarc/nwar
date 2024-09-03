#version 450

layout(binding = 0) uniform FrameUniform {
    mat4 model_view_matrix;
    mat4 projection_matrix;
} frame;

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_uv;

layout(location = 0) out VS_OUT {
    vec2 uv;
    vec3 color_factor;
    vec3 color_offset;
} vs_out;

const vec3 light_vector = normalize(vec3(-1100, 1200, 1000));
const vec3 ambient_factor = vec3(0.4);
const vec3 specular_factor = vec3(0.1);
const float specular_power = 48;

void main() {
    vec4 position_in_view = frame.model_view_matrix * vec4(in_position, 1);
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
