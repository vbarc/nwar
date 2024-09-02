#version 450

layout(location = 0) in VS_OUT {
    vec2 uv;
    vec3 color_factor;
    vec3 color_offset;
} fs_in;

layout(location = 0) out vec4 out_color;

layout(binding = 1) uniform sampler2D tex_sampler;

void main() {
    out_color = texture(tex_sampler, fs_in.uv);
}
