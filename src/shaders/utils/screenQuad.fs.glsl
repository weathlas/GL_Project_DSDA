#version 330

in vec2 vTexCoords;

out vec4 fFragColor;

uniform sampler2D uBaseColorTexture;

void main()
{
    // fFragColor = vec4(vTexCoords.x, vTexCoords.y, 0, 1);
    // fFragColor = vec4(1, 0, 0, 1);
    fFragColor.rgb = vec3(texture(uBaseColorTexture, vTexCoords));
    fFragColor.a = 1.0;
} 