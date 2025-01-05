#version 330

in vec3 normal;

out vec4 fFragColor;

void main() {
    fFragColor.rgb =  normalize(normal);
    fFragColor.a = 0.5f;
}