#version 330

in float sDepth;

out vec4 fFragColor;

void main() {
    fFragColor.rgb =  vec3(1/(1+sDepth));
    fFragColor.a = 0.5f;
}