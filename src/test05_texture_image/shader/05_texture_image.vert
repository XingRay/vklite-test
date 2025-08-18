#version 460
layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec2 inTextureCoordinates;

layout (push_constant) uniform MvpMatrix {
    mat4 mvp; // MVP 矩阵
} mvpMatrix;

layout (location = 0) out vec2 outTextureCoordinates;

void main() {
    gl_Position = mvpMatrix.mvp * vec4(inPosition, 1.0);
    outTextureCoordinates = inTextureCoordinates;
}