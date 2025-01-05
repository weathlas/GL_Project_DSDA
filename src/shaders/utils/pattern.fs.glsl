#version 330

in vec2 vUV;
in vec3 normal;
in vec3 wPosition;


uniform sampler2D uBaseTexture;
uniform sampler2D uAlternateTexture;
uniform vec3 uCameraPosition;

uniform sampler2D uLightDepthMap;

uniform mat3 uLights[256];
uniform int uLightsCount;

out vec4 fFragColor;

vec3 viewDir = normalize(uCameraPosition - wPosition);

vec3 realNormal = normalize(normal);

in vec3 screenPos;

void main()
{
    fFragColor.r = float(int(screenPos.x * 17) % 7)/7.0;
    fFragColor.g = float(int(screenPos.y * 17) % 7)/7.0;
    fFragColor.b = float(int(screenPos.z * 17) % 7)/7.0;
    // would stop some optimisations
    // gl_FragDepth = gl_FragCoord.z;
} 