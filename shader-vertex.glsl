#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 model_view;
    mat4 proj;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUv;

layout(location = 0) out vec3 outNormal;
layout(location = 1) out vec2 outUv;

void main() {
    gl_Position = ubo.proj * ubo.model_view * vec4(inPosition, 1.0);
    outNormal = inNormal;
    outUv = inUv;
}
