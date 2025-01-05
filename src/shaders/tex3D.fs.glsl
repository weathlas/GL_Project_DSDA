#version 330

in vec2 vUV;

uniform sampler2D uTexture;

out vec3 fFragColor;

void main() {
    fFragColor =  texture(uTexture, vUV).xyz;
}