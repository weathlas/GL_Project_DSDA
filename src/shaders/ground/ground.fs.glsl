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

vec3 diffuseFromLight(vec3 lightPos, vec3 lightColor, float maxDistance) {

    vec3 lightVector = lightPos - wPosition;

    vec3 lightDir = normalize(lightVector);
    float diff = max(dot(realNormal, lightDir), 0.0);
    return diff * lightColor * max(0.0, (maxDistance-length(lightVector))/maxDistance);
}

vec3 specularFromLight(vec3 lightPos, vec3 lightColor, float maxDistance) {
    vec3 lightVector = lightPos - wPosition;

    vec3 lightDir = normalize(lightVector);
    vec3 reflectDir = reflect(-lightDir, realNormal);  

    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 128);
    return spec * lightColor * max(0.0, (maxDistance-length(lightVector))/maxDistance);
}

void main() {

    vec3 specularSum = vec3(0);
    vec3 diffuseSum = vec3(0);

    float ambientStrength = 1.0;
    float specularStrength = 3.0 * max((texture(uAlternateTexture, vec2(vUV.x, 1 - vUV.y)).x), 0.0);
    float diffuseStrength = 1.0 * max(1-(texture(uAlternateTexture, vec2(vUV.x, 1 - vUV.y)).x), 0.0);

    vec3 combinedTextureColor = texture(uBaseTexture, vec2(vUV.x, 1 - vUV.y)).xyz;
    
    vec3 p = lightViewPosition.xyz / lightViewPosition.w;
    vec3 shadowColor = texture(uLightDepthMap, p.xy).r < p.z ? vec3(0.02, 0.02, 0.1) : vec3(1);

    float dotProduct = dot(normalize(normal), normalize(vec3(1, 1, -1)));
    float alignedWithSun = dotProduct > 0 ? 1.0 : max(0.0, 1 + dotProduct*4);
    // float alignedWithSun = min(1, dot(normalize(normal), normalize(vec3(1, 1, -1)))+0.25);

    vec3 ambient = vec3(ambientStrength * alignedWithSun) * shadowColor;


    for (int i = 0; i < uLightsCount; i++) {
        diffuseSum += diffuseFromLight(uLights[i][0], uLights[i][1], uLights[i][2].x) * uLights[i][2].y;
        specularSum += specularFromLight(uLights[i][0], uLights[i][1], uLights[i][2].x) * uLights[i][2].y;
    }

    fFragColor.rgb =  (ambient + specularStrength*specularSum + diffuseStrength*diffuseSum) * combinedTextureColor;
    fFragColor.a = max(0.25, texture(uBaseTexture, vec2(vUV.x, 1 - vUV.y)).a);


    // fFragColor.r = texture(uLightDepthMap, vec2(vUV.x, 1 - vUV.y)).r;
    // fFragColor.g = texture(uLightDepthMap, vec2(vUV.x, 1 - vUV.y)).r;
    // fFragColor.b = texture(uLightDepthMap, vec2(vUV.x, 1 - vUV.y)).r;
}