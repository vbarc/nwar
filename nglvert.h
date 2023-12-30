#pragma once

static const char* gVertexShaderSrc = R"(
#version 460 core

layout (location=0) in vec3 pos;
layout (std140, binding = 0) uniform FrameUniform {
    mat4 mvp;
};

void main() {
    gl_Position = mvp * vec4(pos, 1);
}
)";
