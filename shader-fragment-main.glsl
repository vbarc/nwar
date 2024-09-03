#version 450

layout(location = 0) in VS_OUT {
    vec2 uv;
    vec3 color_factor;
    vec3 color_offset;
} fs_in;

layout(location = 0) out vec4 out_color;

layout(binding = 1) uniform sampler2D texture_sampler;

void main() {
    vec4 color = texture(texture_sampler, fs_in.uv);
    out_color = color * vec4(fs_in.color_factor, 1) + vec4(fs_in.color_offset, 1);
}
