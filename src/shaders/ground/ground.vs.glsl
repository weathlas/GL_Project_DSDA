#version 330

layout(location = 0) in vec3 aVertexPosition;
layout(location = 1) in vec3 aVertexNormal;
layout(location = 2) in vec2 aVertexTexture;

uniform mat4 uMVPMatrix;
uniform mat4 uMVMatrix;
uniform mat4 uNormalMatrix;
uniform mat4 uModelMatrix;
uniform vec3 uCameraPosition;

uniform mat4 uShadowMatrix;

out vec2 vUV;
out vec3 normal;
out vec4 wPosition;

out vec4 lightViewPosition;

void main() {
    
    wPosition = (uModelMatrix * vec4(aVertexPosition, 1));

    lightViewPosition = uShadowMatrix * uModelMatrix * vec4(aVertexPosition, 1);
    // vec4 temp = (uModelMatrix * vec4(aVertexPosition, 1));
    // mat3 tempMat3 = mat3(uModelMatrix);
    // vec4 temp = (uModelMatrix * vec4(aVertexPosition, 1));
    // temp += (uModelMatrix * vec4(- uCameraPosition, 1));
    // temp = temp + vec4(tempMat3 * uCameraPosition * temp.w, temp.w);
    // lightViewPosition = uShadowMatrix * (temp - temp.w*vec4(uCameraPosition+vec3(0, 1.78, 0), 1));
    // lightViewPosition = uShadowMatrix * (temp - temp.w*vec4(uCameraPosition, 1));
    // lightViewPosition = uShadowMatrix * (temp+vec4(uCameraPosition, 1));// + uShadowMatrix * temp.w*vec4(uCameraPosition, 1);
    
    // lightViewPosition = uShadowMatrix * temp;
    // lightViewPosition = uShadowMatrix * temp - vec4(tempMat3 * uCameraPosition, 1);
    // lightViewPosition.xyz -= uCameraPosition.xyz;

    // vColor = (uNormalMatrix * vec4(aVertexNormal, 0)).rgb;
    vUV = aVertexTexture;
    // vColor = vec3(aVertexTexture.xy, 0);

    vec4 output = uMVPMatrix * vec4(aVertexPosition, 1);
    // gl_Position = vec4(uMVMatrix * uMVPMatrix * vec4(aVertexPosition, 1));
    // gl_Position = vec4(0.1*(aVertexPosition.x/(1+aVertexPosition.z)), 0.1*(aVertexPosition.y/(1+aVertexPosition.z)), 0, 1);
    gl_Position = output;


    normal = normalize(mat3(uModelMatrix) * aVertexNormal);
    // mat4 normalMatrix = transpose(inverse((uModelMatrix)));
    // normal = normalize(vec4(normal, 1) * normalMatrix).xyz;

    // normal = aVertexNormal;

    // vec4 axis = vec4(wPosition - uCameraPosition, 0);
    // float lengthAxis = length(axis);
    // gl_Position = output - vec4(0, 0.5, 0, 0) * lengthAxis;// + 0.0001*(( 1/lengthAxis) * normalize(axis));
    // wPosition = (uMVMatrix * vec4(aVertexPosition, 1)).xyz;
    // wPosition = (vec4(aVertexPosition, 1) * uModelMatrix).xyz;
    // wPosition = aVertexPosition;
}