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

void main() {
    gl_Position = frame.projection_matrix * frame.model_view_matrix * vec4(in_position, 1.0);
    vs_out.uv = in_uv;
}
