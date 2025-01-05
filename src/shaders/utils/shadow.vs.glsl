#version 330

layout(location = 0) in vec3 aVertexPosition;

uniform mat4 uMVPMatrix; // really the model view ligth projection

void main() {
    gl_Position = uMVPMatrix * vec4(aVertexPosition, 1.0);
}