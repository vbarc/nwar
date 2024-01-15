#pragma once

static const char* gGeometryShaderSrc = R"(
#version 460 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in VS_OUT {
    vec2 uv;
    vec3 diffuse_factor;
    vec3 specular_component;
} gs_in[];

out GS_OUT {
    vec2 uv;
    vec3 diffuse_factor;
    vec3 specular_component;
    vec3 barycoords;
} gs_out;

const vec3 reference_barycoords[] = vec3[] (
    vec3(1.0, 0.0, 0.0),
    vec3(0.0, 1.0, 0.0),
    vec3(0.0, 0.0, 1.0)
);

void main() {
    for (int i = 0; i < 3; i++) {
        gl_Position = gl_in[i].gl_Position;
        gs_out.uv = gs_in[i].uv;
        gs_out.diffuse_factor = gs_in[i].diffuse_factor;
        gs_out.specular_component = gs_in[i].specular_component;
        gs_out.barycoords = reference_barycoords[i];
        EmitVertex();
    }
    EndPrimitive();
}
)";
