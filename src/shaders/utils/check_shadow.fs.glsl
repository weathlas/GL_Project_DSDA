#version 330

in vec2 vUV;
in vec3 normal;
in vec3 wPosition;

// uniform mat4 uModelMatrixLoc;

// uniform sampler2D uBaseTexture;
// uniform sampler2D uAlternateTexture;
uniform vec3 uCameraPosition;

uniform sampler2D uLightDepthMap;

// uniform mat3 uLights[256];
// uniform int uLightsCount;

out vec4 fFragColor;

// vec3 viewDir = normalize(uCameraPosition - wPosition);

// vec3 realNormal = normalize(normal);

void main() {
    // fFragColor.r = 0*(texture(uBaseTexture, vec2(vUV.x, 1 - vUV.y)).r);
    // fFragColor.g = 0*(texture(uAlternateTexture, vec2(vUV.x, 1 - vUV.y)).r);
    // fFragColor.b = 1*(texture(uLightDepthMap, vec2(vUV.x, 1 - vUV.y)).b);
    fFragColor.r = texture(uLightDepthMap, vec2(vUV.x, 1 - vUV.y)).r;
    fFragColor.gb = vec2(0);
}