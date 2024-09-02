#version 450

layout(location = 0) in VS_OUT {
    vec2 uv;
    vec3 color_factor;
    vec3 color_offset;
} fs_in;

layout(location = 0) out vec4 out_color;

void main() {
    out_color = vec4(1, 0, 0, 1);
}
