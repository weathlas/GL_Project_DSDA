#version 330

layout(location = 0) in vec3 aVertexPosition;
layout(location = 1) in vec3 aVertexNormal;
layout(location = 2) in vec2 aVertexTexture;

uniform mat4 uMVPMatrix;
uniform mat4 uMVMatrix;
uniform mat4 uNormalMatrix;
uniform mat4 uModelMatrix;

uniform vec3 uCameraPosition;
uniform vec3 uCameraFront;

out vec3 wPosition;
out vec3 normal;

// out float selected;

void main() {

    vec4 output = uMVPMatrix * vec4(aVertexPosition, 1);
    gl_Position = output;

    wPosition = (uModelMatrix * vec4(aVertexPosition, 1)).xyz;

    normal = normalize(mat3(uModelMatrix) * aVertexNormal);

    
    // vec3 direction = wPosition - uCameraPosition;
    // float dist = length(direction) <= 1.5 ? 1.0 : 0.0;

    // selected = dist;

    // selected = abs(dot(normalize(uCameraFront) , normalize(direction)))> 0.9995?dist:0.0;
}