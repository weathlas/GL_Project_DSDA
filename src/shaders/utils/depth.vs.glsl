#version 330

layout(location = 0) in vec3 aVertexPosition;
layout(location = 1) in vec3 aVertexNormal;
layout(location = 2) in vec2 aVertexTexture;

uniform mat4 uMVPMatrix;

out float sDepth;

void main() {

    vec4 output = uMVPMatrix * vec4(aVertexPosition, 1);
    gl_Position = output;

    sDepth = output.w;
}