#version 330

in vec2 vUV;
in vec3 normal;
in vec4 wPosition;

in vec4 lightViewPosition;

// uniform mat4 uModelMatrixLoc;

uniform sampler2D uBaseTexture;
uniform sampler2D uAlternateTexture;
uniform sampler2D uLightDepthMap;

uniform vec3 uCameraPosition;
uniform vec2 uWindowDimensions;

uniform float uClippingPlaneActive;
uniform vec4 uClippingPlane;

uniform vec3 uSunLightPos;

uniform mat3 uLights[200];
uniform int uLightsCount;

out vec4 fFragColor;

vec3 viewDir = normalize(uCameraPosition - wPosition.xyz);

vec3 realNormal = normalize(normal);

vec3 diffuseFromLight(vec3 lightPos, vec3 lightColor, float maxDistance) {

    vec3 lightVector = lightPos - wPosition.xyz;

    vec3 lightDir = normalize(lightVector);
    float diff = max(dot(realNormal, lightDir), 0.0);
    return diff * lightColor * max(0.0, (maxDistance-length(lightVector))/maxDistance);
}

vec3 specularFromLight(vec3 lightPos, vec3 lightColor, float maxDistance) {
    vec3 lightVector = lightPos - wPosition.xyz;

    vec3 lightDir = normalize(lightVector);
    vec3 reflectDir = reflect(-lightDir, realNormal);  

    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 128);
    return spec * lightColor * max(0.0, (maxDistance-length(lightVector))/maxDistance);
}

void main() {

    float dist = 0.0;
    if(uClippingPlaneActive > 0.0) {
        dist = dot(uClippingPlane, wPosition);
        if (dist < 0.0) {
            discard;
        }
    }

    // if(uClippingPlaneActive > 0.0) {
    //     discard;
    // }

    vec3 specularSum = vec3(0);
    vec3 diffuseSum = vec3(0);

    float ambientStrength = 1.0;
    float specularStrength = 3.0 * max((texture(uAlternateTexture, vec2(vUV.x, 1 - vUV.y)).x), 0.0);
    float diffuseStrength = 1.0 * max(1-(texture(uAlternateTexture, vec2(vUV.x, 1 - vUV.y)).x), 0.0);

    vec3 combinedTextureColor = texture(uBaseTexture, vec2(vUV.x, 1 - vUV.y)).xyz;
    
    vec3 p = lightViewPosition.xyz / lightViewPosition.w;

    bool isHiddenFromSun = false;
    if(p.x >= 0.0 && p.x <= 1.0 && p.y >= 0.0 && p.y <= 1.0) {
        isHiddenFromSun = texture(uLightDepthMap, p.xy).r < p.z;
    }

    vec3 shadowColor = isHiddenFromSun ? vec3(0.02, 0.02, 0.1) : vec3(1);
    // shadowColor = vec3(1);

    // float dotProduct = dot(normalize(normal), normalize(vec3(1, 1, -1)));
    float dotProduct = dot(normalize(normal), normalize(uSunLightPos));
    float alignedWithSun = dotProduct > 0 ? 1.0 : max(0.0, 1 + dotProduct*3);
    // float alignedWithSun = min(1, dot(normalize(normal), normalize(vec3(1, 1, -1)))+0.25);

    vec3 ambient = vec3(ambientStrength * alignedWithSun) * shadowColor;


    for (int i = 0; i < uLightsCount; i++) {
        diffuseSum += diffuseFromLight(uLights[i][0], uLights[i][1], uLights[i][2].x) * uLights[i][2].y;
        specularSum += specularFromLight(uLights[i][0], uLights[i][1], uLights[i][2].x) * uLights[i][2].y;
    }

    fFragColor.rgb =  (ambient + specularStrength*specularSum + diffuseStrength*diffuseSum) * combinedTextureColor;
    fFragColor.a = max(0.06, texture(uBaseTexture, vec2(vUV.x, 1 - vUV.y)).a);

    vec2 offseted = gl_FragCoord.xy - uWindowDimensions/2;
    // vec2 offseted = gl_FragCoord.xy - vec2(int(uWindowDimensions.x/2), int(uWindowDimensions.y/2));
    // vec2 offseted = vec2(gl_FragCoord.x - uWindowDimensions.x/2, gl_FragCoord.y - uWindowDimensions.y/2);

    float distanceToCenter = length(offseted);

    // fFragColor.rgb = vec3(isHiddenFromSun?1.0:0.0, isHiddenFromSun?1.0:0.0, isHiddenFromSun?1.0:0.0);


    // fFragColor.r = texture(uLightDepthMap, p.xy).r;
    // fFragColor.g = (p.x >= 0.0 && p.x <= 1.0)?fFragColor.g:1.0;
    // fFragColor.b = (p.y >= 0.0 && p.y <= 1.0)?fFragColor.b:1.0;

    // fFragColor *= (p.x >= 0.0 && p.x <= 1.0 && p.y >= 0.0 && p.y <= 1.0)?0.0:1.0;


    if (distanceToCenter > uWindowDimensions.x/2) {
        int id = int(distanceToCenter) % 3;

        float fac = clamp(1.0 - (distanceToCenter-(uWindowDimensions.x/2)) / (uWindowDimensions.x/4), 0.0, 1.0);

        fFragColor.r *= (id == 0)?1.0:fac;
        fFragColor.g *= (id == 1)?1.0:fac;
        fFragColor.b *= (id == 2)?1.0:fac;
        fFragColor.rgb *= fac;
        // fFragColor.rgb = fFragColor.rgb;

    }

    // if(length(offseted) >450) {
    //     fFragColor.rgb *= 0;
    //     fFragColor.a = 1.0;
    // }

    // fFragColor.r = texture(uLightDepthMap, vec2(vUV.x, 1 - vUV.y)).r;
    // fFragColor.g = texture(uLightDepthMap, vec2(vUV.x, 1 - vUV.y)).r;
    // fFragColor.b = texture(uLightDepthMap, vec2(vUV.x, 1 - vUV.y)).r;
}