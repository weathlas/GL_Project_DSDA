#version 330

in vec2 vUV;
in vec3 normal;
in vec3 wPosition;

in vec4 lightViewPosition;

// uniform mat4 uModelMatrixLoc;

uniform sampler2D uBaseTexture;
uniform sampler2D uAlternateTexture;
uniform sampler2D uLightDepthMap;

uniform vec3 uCameraPosition;


uniform mat3 uLights[256];
uniform int uLightsCount;

out vec4 fFragColor;

vec3 viewDir = normalize(uCameraPosition - wPosition);

vec3 realNormal = normalize(normal);

// vec3 diffuseFromLight(vec3 lightPos, vec3 lightColor, float maxDistance) {

//     vec3 lightVector = lightPos - wPosition;

//     vec3 lightDir = normalize(lightVector);
//     float diff = max(dot(realNormal, lightDir), 0.0);
//     return diff * lightColor * max(0.0, (maxDistance-length(lightVector))/maxDistance);
// }

// vec3 specularFromLight(vec3 lightPos, vec3 lightColor, float maxDistance) {
//     vec3 lightVector = lightPos - wPosition;

//     vec3 lightDir = normalize(lightVector);
//     vec3 reflectDir = reflect(-lightDir, realNormal);  

//     float spec = pow(max(dot(viewDir, reflectDir), 0.0), 128);
//     return spec * lightColor * max(0.0, (maxDistance-length(lightVector))/maxDistance);
// }

void main() {
    
    vec3 p = lightViewPosition.xyz / lightViewPosition.w;

    p = 0.5 * (p.xyz + vec3(1.0));

    // vec3 colorAtMirrorView = texture(uLightDepthMap,  vec2(p.x*3, 1-p.y*3)).rgb;
    vec3 colorAtMirrorView = texture(uLightDepthMap,  vec2(-p.x, -p.y)).rgb;
    // vec3 colorAtMirrorView = texture(uLightDepthMap,  vec2(0)).rgb;
    // vec3 colorAtMirrorView = texture(uLightDepthMap,  vec2(p.x*1600, p.y*900)).rgb;
    // vec3 colorAtMirrorView = texture(uLightDepthMap, vec2(p.x*1600, p.y*900)).rgb;
    // vec3 colorAtMirrorView = texture(uLightDepthMap, vUV).rgb;

    // fFragColor.rgb = p.xyz;

    fFragColor.rgb = colorAtMirrorView;
}