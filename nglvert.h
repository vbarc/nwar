#pragma once

static const char* gVertexShaderSrc = R"(
#version 460 core

layout (location=0) in vec3 pos;
layout (location=1) in vec3 barypos_in;
layout (location=0) out vec3 barypos;
layout (std140, binding = 0) uniform FrameUniform {
    mat4 mvp;
};

void main() {
    barypos = barypos_in;
    gl_Position = mvp * vec4(pos, 1);
}
)";
