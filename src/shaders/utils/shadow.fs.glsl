#version 330

// layout(location = 0) out float fragmentDepth;

// in vec2 vUV;
// in vec3 normal;
// in vec3 wPosition;


// uniform sampler2D uLightDepthMap;

// uniform sampler2D uBaseTexture;
// uniform sampler2D uAlternateTexture;
// uniform vec3 uCameraPosition;

// uniform sampler2D uLightDepthMap;

// uniform mat3 uLights[256];
// uniform int uLightsCount;

// in vec3 screenPos;

out vec4 fFragColor;

void main()
{
    // fFragColor.r = (int(screenPos.x * 100) % 7)/7;
    // fFragColor.g = (int(screenPos.y * 100) % 7)/7;
    // fFragColor.b = (int(screenPos.z * 100) % 7)/7;
    // fFragColor.r = (int(gl_FragCoord.z*10)%10)/10.0f;
    // fFragColor.r = screenPos.r / screenPos.b;
    // fFragColor.g = screenPos.g / screenPos.b;
    // fFragColor.b = screenPos.b;
    // would stop some optimisations

    fFragColor = vec4(vec3(gl_FragCoord.z), 1.0);
    // gl_FragDepth = gl_FragCoord.z;
    gl_FragDepth = 0;
} 