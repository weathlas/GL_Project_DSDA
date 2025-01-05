#version 330

layout(location = 0) out vec4 color;

uniform sampler2D uBaseTexture;
uniform sampler2D uAlternateTexture;
uniform sampler2D uLightDepthMap;

in vec4 lightViewPosition;
in vec2 vUV;
in vec3 normal;

void main() {


    // dot(normal, normalize(vec3(1, 1, 1))) > 0


    color = vec4(texture(uBaseTexture, vec2(vUV.x, 1 - vUV.y)).rgb, 1);
    vec3 p = lightViewPosition.xyz / lightViewPosition.w;
    color *= texture(uLightDepthMap, p.xy).r < p.z ? 0 : 1;

    color *= dot(normalize(normal), normalize(vec3(1, 1, -1))) > 0 ? 1.0 : 0.0;
    // color.rgb *= texture(uBaseTexture, p.xy).r;
    // color.rgb *= texture(uAlternateTexture, p.xy).r;
    // color.rgb *= texture(uLightDepthMap, p.xy).r+0.1;
    // color.rgb *= texture(uLightDepthMap, p.xy).r+0.1;
    // float val = texture(uLightDepthMap, p.xy).r;
    // color.rgb *= max(0, (1-pow(val, 16)));
    // float val = texture(uLightDepthMap, vUV).r;
    // color.rgb *= texture(uLightDepthMap, vUV).r;
    // color.rgb *= max(0, (1-pow(val, 1)));
    // color.rgb *= val;
}