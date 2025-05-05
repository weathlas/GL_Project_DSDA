#version 330

out vec4 fFragColor;

void main()
{
    // fFragColor = vec4(vec3(gl_FragCoord.z), 1.0);
    fFragColor.rgba =  vec4(1.0);
} 