#version 330

out vec4 fFragColor;

in vec3 pos;

void main()
{
    fFragColor = vec4(1, 1, 1, 1.0);
    // gl_FragDepth = 0;
} 