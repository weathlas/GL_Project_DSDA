#version 330

layout(location = 0) in vec3 aVertexPosition;

uniform mat4 uMVPMatrix; // really the model view ligth projection

out vec3 pos;

void main() {
    gl_Position = uMVPMatrix * vec4(aVertexPosition, 1.0);
    pos = gl_Position.xyz;
}