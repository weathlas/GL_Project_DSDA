#version 330

in vec3 wPosition;
in vec3 normal;
// in float selected;

uniform sampler2D uBaseTexture;
uniform sampler2D uAlternateTexture;
uniform vec3 uCameraPosition;
uniform vec3 uCameraFront;

uniform mat3 uLights[200];
uniform int uLightsCount;

out vec4 fFragColor;

vec3 realNormal = normalize(normal);

void main() {

    vec3 direction = wPosition - uCameraPosition;
    float dotResult = dot(normalize(direction) , realNormal);
    // fFragColor.rgb = vec3(0.69, 0.867, 0.867);
    // if(dotResult < 0.35 && dotResult > -0.35) {
    //     fFragColor.rgb = vec3(1.0);
    // }
    float dist = length(direction) <= 1.5 ? 1.0 : 0.0;
    float selected = abs(dot(normalize(uCameraFront) , normalize(direction)))> 0.9995?dist:0.0;

    fFragColor.rgb = selected>0.0?vec3(0.89, 0.788, 0.373):vec3(0.69, 0.867, 0.867) * pow(1 - abs(dotResult), 3);
    // fFragColor.rgb = selected>0.1?vec3(0, 1, 0):vec3(1, 0, 0);

    // if(length(uCameraFront) < 0.5) {
    //     fFragColor.rgb = vec3(1, 0, 0);
    // }
    // fFragColor.a = pow(1 - abs(dotResult), 3);
    fFragColor.a = 0.5;
}