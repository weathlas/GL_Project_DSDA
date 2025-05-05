#version 330

in vec3 normal;

out vec4 fFragColor;

void main() {
    // fFragColor.rgb =  normalize(normal);
    // fFragColor.rgb = vec3(1, 0, 0);
    // fFragColor.a = 1.0;
    fFragColor.rgb =  (normalize(normal) * 0.5 + vec3(0.5));
    fFragColor.a = 0.5f;
}