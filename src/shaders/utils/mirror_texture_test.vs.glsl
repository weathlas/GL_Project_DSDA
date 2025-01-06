#version 330

layout(location = 0) in vec3 aVertexPosition;
layout(location = 1) in vec3 aVertexNormal;
layout(location = 2) in vec2 aVertexTexture;

uniform mat4 uMVPMatrix;
uniform mat4 uShadowMatrix;
uniform mat4 uModelMatrix;

out vec4 lightViewPosition;
out vec2 vUV;
out vec3 normal;

void main() {
    gl_Position = uMVPMatrix * vec4(aVertexPosition, 1);
    lightViewPosition = uShadowMatrix * uModelMatrix * vec4(aVertexPosition, 1);
    vUV = aVertexTexture;
    normal = normalize(mat3(uModelMatrix) * aVertexNormal);
}