#version 460
layout (location = 0) in vec3 inPosition;

layout (set = 0, binding = 0) uniform ColorUniformBufferObject {
    vec3 color;
} colorUbo;

layout (location = 0) out vec3 fragColor;

void main() {
    gl_Position = vec4(inPosition, 1.0);
    fragColor = colorUbo.color;
}
