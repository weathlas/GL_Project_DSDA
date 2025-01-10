#version 330

in vec4 wPosition;

uniform sampler2D uBaseTexture;
uniform sampler2D uAlternateTexture;
uniform vec3 uCameraPosition;

uniform float uClippingPlaneActive;
uniform vec4 uClippingPlane;

uniform mat3 uLights[200];
uniform int uLightsCount;

out vec4 fFragColor;

vec3 getColor() {
    int index = -1;
    float minDist = 0.05;
    for (int i = 0; i < uLightsCount; i++) {
        float dist = length(uLights[i][0] - wPosition.xyz);
        if (index == -1 || dist < minDist) {
            index = i;
            minDist = dist;
        }

    }
    if (index >= 0) {
        return uLights[index][1];
    }
    return vec3(0);
}

void main() {
    if(uClippingPlaneActive > 0.0) {
        if (dot(uClippingPlane, wPosition) < 0.0) {
            discard;
        }
    }
    fFragColor.rgb = getColor();
    fFragColor.a = 0.5f;
}