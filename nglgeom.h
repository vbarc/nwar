#pragma once

static const char* gGeometryShaderSrc = R"(
#version 460 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;
layout (location = 0) in vec4 vsout_color[];
layout (location = 0) out vec4 gsout_color;
layout (location = 1) out vec3 gsout_barycoords;

const vec3 reference_barycoords[] = vec3[] (
    vec3(1.0, 0.0, 0.0),
    vec3(0.0, 1.0, 0.0),
    vec3(0.0, 0.0, 1.0)
);

void main() {
    for (int i = 0; i < 3; i++) {
        gl_Position = gl_in[i].gl_Position;
        gsout_color = vsout_color[i];
        gsout_barycoords = reference_barycoords[i];
        EmitVertex();
    }
    EndPrimitive();
}
)";
