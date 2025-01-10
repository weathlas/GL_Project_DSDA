#version 330

in vec2 vUV;
in vec3 normal;
in vec4 wPosition;
// in vec3 tangent;

in vec4 lightViewPosition;

in mat3 TBN;

// uniform mat4 uModelMatrixLoc;

uniform sampler2D uBaseTexture;
uniform sampler2D uAlternateTexture;
uniform sampler2D uLightDepthMap;
uniform sampler2D uNormalMap;

uniform vec3 uCameraPosition;
uniform vec2 uWindowDimensions;

uniform float uClippingPlaneActive;
uniform vec4 uClippingPlane;

uniform vec3 uSunLightPos;

uniform mat3 uLights[200];
uniform int uLightsCount;

out vec4 fFragColor;

vec3 normalTangentSpace(vec2 coords) {
    return normalize(texture(uNormalMap, coords).rgb * 2.0 - 1.0);  
}

vec3 lightDirTangentSpace(vec3 lightDirection) {
    return TBN * normalize(lightDirection);
}

vec3 viewDir = normalize(uCameraPosition - wPosition.xyz);

// vec3 realNormal = normalize(normal);
vec3 realNormal = normalize(lightDirTangentSpace(normalTangentSpace(vec2(vUV.x, 1 - vUV.y))));
// float diffuseFromSun = max(dot(normalTangentSpace(vec2(vUV.x, 1 - vUV.y)), lightDirTangentSpace(wPosition.xyz-uSunLightPos)), 0.0);

float Ka = 1.0;
float Kd = 1.0;
float Ks = 1.0;

// attenuate is used with the sun which does not nned attenuation
vec3 diffuseFromLight(vec3 lightPos, vec3 lightColor, float maxDistance, float attenuate) {

    // wPosition.xyz*attenuate is used to make the lightVector == wPosition
    vec3 lightVector = lightPos - wPosition.xyz*attenuate;

    vec3 lightDir = normalize(lightVector);

    float diff = max(dot(realNormal, lightDir), 0.0);

    // float diff = max(dot(normalTangentSpace(vec2(vUV.x, 1 - vUV.y)), lightDirTangentSpace(lightDir)), 0.0);
    return diff * lightColor * max(0.0, (1.0-attenuate) + attenuate * (maxDistance-length(lightVector))/maxDistance);
}

vec3 specularFromLight(vec3 lightPos, vec3 lightColor, float maxDistance, float attenuate) {

    // wPosition.xyz*attenuate is used to make the lightVector == wPosition
    vec3 lightVector = lightPos - wPosition.xyz*attenuate;

    vec3 lightDir = normalize(lightVector);
    vec3 reflectDir = reflect(-lightDir, realNormal);  

    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 128);
    return spec * lightColor * max(0.0, (1.0-attenuate) + attenuate * (maxDistance-length(lightVector))/maxDistance);
}

void main() {

    // mirror stuff
    float dist = 0.0;
    if(uClippingPlaneActive > 0.0) {
        dist = dot(uClippingPlane, wPosition);
        if (dist < 0.0) {
            discard;
        }
    }


    vec3 specularSum = vec3(0);
    vec3 diffuseSum = vec3(0);
    vec3 ambient = vec3(0);

    float specularStrength = Ks * max(1-(texture(uAlternateTexture, vec2(vUV.x, 1 - vUV.y)).x), 0.0);
    // specularStrength = 3.0;
    float diffuseStrength = Kd * 1.0;// * max(1-(texture(uAlternateTexture, vec2(vUV.x, 1 - vUV.y)).x), 0.0);

    vec3 combinedTextureColor = texture(uBaseTexture, vec2(vUV.x, 1 - vUV.y)).xyz;
    

    vec3 p = lightViewPosition.xyz / lightViewPosition.w;
    float haveLightFromSun = 0.0;
    if(p.x >= 0.0 && p.x <= 1.0 && p.y >= 0.0 && p.y <= 1.0) {
        haveLightFromSun = texture(uLightDepthMap, p.xy).r > p.z ? 1.0:0.0;
    }

    vec3 sunColor = vec3(1, 1, 1);
    float diffuseFromSun = diffuseFromLight(uSunLightPos, sunColor, 0.0, 0.0).r;
    vec3 specularFromSun = specularFromLight(uSunLightPos, sunColor, 0.0, 0.0);
    // diffuseSum += diffuseFromSun;
    // specularSum += 

    // haveLightFromSun = 0.0;

    vec3 colorFromSun = (sunColor * diffuseFromSun) + (vec3(0.02, 0.02, 0.1) * (1 - diffuseFromSun));

    // vec3 colorFromSun = diffuseFromSun > 0.0 ? vec3(1) : vec3(0.02, 0.02, 0.1);

    // vec3 shadowColor = haveLightFromSun ? vec3(0.02, 0.02, 0.1) : vec3(1);
    // shadowColor = vec3(1);

    // float dotProduct = dot(normalize(normal), normalize(vec3(1, 1, -1)));
    float dotProduct = dot(normalize(normal), normalize(uSunLightPos));
    float alignedWithSun = dotProduct > 0 ? 1.0 : max(0.0, 1 + dotProduct*3);
    // float alignedWithSun = min(1, dot(normalize(normal), normalize(vec3(1, 1, -1)))+0.25);

    // ambient += vec3(Ka * alignedWithSun) * colorFromSun * haveLightFromSun;
    diffuseSum += colorFromSun * haveLightFromSun;
    specularSum += specularFromSun * haveLightFromSun;


    for (int i = 0; i < uLightsCount; i++) {
        diffuseSum += diffuseFromLight(uLights[i][0], uLights[i][1], uLights[i][2].x, 1.0) * uLights[i][2].y;
        specularSum += specularFromLight(uLights[i][0], uLights[i][1], uLights[i][2].x, 1.0) * uLights[i][2].y;
    }

    fFragColor.rgb =  (ambient + specularStrength*specularSum + diffuseStrength*diffuseSum) * combinedTextureColor;
    fFragColor.a = max(0.00, texture(uBaseTexture, vec2(vUV.x, 1 - vUV.y)).a);

    // Corner RGB effect
    vec2 offseted = gl_FragCoord.xy - uWindowDimensions/2;
    float distanceToCenter = length(offseted);
    int id = int(distanceToCenter) % 3;
    float fac = clamp(1.0 - (distanceToCenter-(uWindowDimensions.x/2)) / (uWindowDimensions.x/4), 0.0, 1.0);
    fFragColor.r *= (id == 0)?1.0:fac;
    fFragColor.g *= (id == 1)?1.0:fac;
    fFragColor.b *= (id == 2)?1.0:fac;
    fFragColor.rgb *= fac;

    // float diffuseFromSun = max(dot(normalTextureCoordsToVector(vec2(vUV.x, 1 - vUV.y)), lightDirTangentSpace(uSunLightPos-wPosition.xyz)), 0.0);

    // fFragColor.rgb = texture(uNormalMap, vec2(vUV.x, 1 - vUV.y)).rgb;

    // fFragColor.rgb = vec3(diffuseFromSun);
    // fFragColor.rgb = tangent;
    // fFragColor.rgb = realNormal;
}