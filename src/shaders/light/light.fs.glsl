#version 330

in vec3 wPosition;


uniform sampler2D uBaseTexture;
uniform sampler2D uAlternateTexture;
uniform vec3 uCameraPosition;

uniform mat3 uLights[330];
uniform int uLightsCount;

out vec4 fFragColor;

vec3 getColor() {
    int index = -1;
    float minDist = 0.05;
    for (int i = 0; i < uLightsCount; i++) {
        float dist = length(uLights[i][0] - wPosition);
        if (dist < minDist) {
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
    fFragColor.rgb = getColor();
    fFragColor.a = 0.5f;
}