#version 330

layout(location = 0) in vec3 aVertexPosition;
layout(location = 1) in vec3 aVertexNormal;
layout(location = 2) in vec2 aVertexTexture;

uniform mat4 uMVPMatrix;

out float sDepth;

void main() {

    gl_Position = uMVPMatrix * vec4(aVertexPosition, 1);

    sDepth = gl_Position.w;
}