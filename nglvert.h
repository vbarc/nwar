#pragma once

static const char* gVertexShaderSrc = R"(
#version 460 core

layout (location=0) in vec3 position;
layout (location=1) in vec3 normal;

layout (std140, binding = 0) uniform FrameUniform {
    mat4 model_view_matrix;
    mat4 projection_matrix;
    int is_wireframe_enabled;
};

void main() {
    gl_Position = projection_matrix * model_view_matrix * vec4(position, 1);
}
)";
