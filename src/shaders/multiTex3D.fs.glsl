#version 330

in vec2 vUV;

uniform sampler2D uBaseTexture;
uniform sampler2D uAlternateTexture;

out vec3 fFragColor;

void main() {
    fFragColor =  texture(uBaseTexture, vUV).xyz + texture(uAlternateTexture, vUV).xyz;
}