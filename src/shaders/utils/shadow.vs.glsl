#version 330

layout(location = 0) in vec3 aVertexPosition;
// layout(location = 1) in vec3 aVertexNormal;
// layout(location = 2) in vec2 aVertexTexture;

// uniform mat4 lightSpaceMatrix;

uniform mat4 uMVPMatrix; // really the model view ligth projection
// uniform mat4 uMVMatrix;
// uniform mat4 uNormalMatrix;
// uniform mat4 uModelMatrix;

// out vec2 vUV;
// out vec3 normal;
// out vec3 wPosition;

// out vec3 screenPos;

void main() {
    // gl_Position = lightSpaceMatrix * uModelMatrix * vec4(aVertexPosition, 1.0);
    gl_Position = uMVPMatrix * vec4(aVertexPosition, 1.0);
    // screenPos = gl_Position.xyz;
}