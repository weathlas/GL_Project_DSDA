#version 330
 
in vec2 vUV;
in vec3 normal;
in vec3 wPosition;

uniform sampler2D uBaseTexture;
uniform sampler2D uAlternateTexture;
uniform vec3 uCameraPosition;

out vec4 fFragColor;
 
void main()
{
    fFragColor.rgb = texture(uBaseTexture, vec2(vUV.x, 1 - vUV.y)).xyz + texture(uAlternateTexture, vec2(vUV.x, 1 - vUV.y)).xyz;
    // gl_FragDepth = 0.999;
    // fFragColor.a = 0.5f;
}