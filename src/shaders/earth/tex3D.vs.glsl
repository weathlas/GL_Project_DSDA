#version 330

layout(location = 0) in vec3 aVertexPosition;
layout(location = 1) in vec3 aVertexNormal;
layout(location = 2) in vec2 aVertexTexture;

uniform mat4 uMVPMatrix;
uniform mat4 uMVMatrix;
uniform mat4 uNormalMatrix;

out vec2 vUV;

void main() {
    // vColor = (uNormalMatrix * vec4(aVertexNormal, 0)).rgb;
    vUV = aVertexTexture;
    // vColor = vec3(aVertexTexture.xy, 0);

    vec4 output = uMVPMatrix * vec4(aVertexPosition, 1);
    // gl_Position = vec4(uMVMatrix * uMVPMatrix * vec4(aVertexPosition.xyz, 1));
    // gl_Position = vec4(0.1*(aVertexPosition.x/(1+aVertexPosition.z)), 0.1*(aVertexPosition.y/(1+aVertexPosition.z)), 0, 1);
    gl_Position = output;
}