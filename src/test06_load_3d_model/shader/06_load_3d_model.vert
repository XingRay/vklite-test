#version 450

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec2 inTexCoord;

layout (set = 0, binding = 0) uniform MvpMatrix {
    mat4 mvp; // MVP 矩阵
} mvpMatrix;

layout (location = 0) out vec2 fragTexCoord;

void main() {
    gl_Position = mvpMatrix.mvp * vec4(inPosition, 1.0);
    fragTexCoord = inTexCoord;
}