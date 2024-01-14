#pragma once

static const char* gGeometryShaderSrc = R"(
#version 460 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;
layout (location = 0) in vec4 vsout_color[];
layout (location = 1) in vec2 vsout_uv[];
layout (location = 2) in int vsout_type[];
layout (location = 3) in vec3 vsout_diffuse_factor[];
layout (location = 4) in vec3 vsout_specular_component[];

layout (location = 0) out vec4 gsout_color;
layout (location = 1) out vec2 gsout_uv;
layout (location = 2) flat out int gsout_type;
layout (location = 3) out vec3 gsout_diffuse_factor;
layout (location = 4) out vec3 gsout_specular_component;
layout (location = 5) out vec3 gsout_barycoords;

const vec3 reference_barycoords[] = vec3[] (
    vec3(1.0, 0.0, 0.0),
    vec3(0.0, 1.0, 0.0),
    vec3(0.0, 0.0, 1.0)
);

void main() {
    for (int i = 0; i < 3; i++) {
        gl_Position = gl_in[i].gl_Position;
        gsout_color = vsout_color[i];
        gsout_uv = vsout_uv[i];
        gsout_type = vsout_type[i];
        gsout_diffuse_factor = vsout_diffuse_factor[i];
        gsout_specular_component = vsout_specular_component[i];
        gsout_barycoords = reference_barycoords[i];
        EmitVertex();
    }
    EndPrimitive();
}
)";
