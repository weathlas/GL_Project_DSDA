#version 330

layout(location = 0) in vec3 aVertexPosition;
layout(location = 1) in vec3 aVertexNormal;
layout(location = 2) in vec2 aVertexTexture;
layout(location = 3) in vec3 aVertexTangeant;

uniform mat4 uMVPMatrix;
uniform mat4 uMVMatrix;
uniform mat4 uNormalMatrix;
uniform mat4 uModelMatrix;
uniform vec3 uCameraPosition;

uniform mat4 uShadowMatrix;

out vec2 vUV;
out vec3 normal;
out vec4 wPosition;
// out vec3 tangent;

out vec4 lightViewPosition;

out mat3 TBN;

void main() {
    
    wPosition = (uModelMatrix * vec4(aVertexPosition, 1));

    lightViewPosition = uShadowMatrix * uModelMatrix * vec4(aVertexPosition, 1);

    vUV = aVertexTexture;

    vec4 output = uMVPMatrix * vec4(aVertexPosition, 1);
    gl_Position = output;


    normal = normalize(mat3(uModelMatrix) * aVertexNormal);
    vec3 tangent = normalize(mat3(uModelMatrix) * aVertexTangeant);
    vec3 bitangent = cross(normal, tangent); // Reconstruct bitangent
    TBN = mat3(tangent, bitangent, normal);

    // vec3 axis = uCameraPosition.xyz - wPosition.xyz;
    // float lengthAxis = length(axis);
    // // gl_Position = output - vec4(0, 0.5, 0, 0) * lengthAxis;// + 0.0001*(( 1/lengthAxis) * normalize(axis));
    // gl_Position = output + vec4(normalize(axis) * (1/lengthAxis), 0);// + 0.0001*(( 1/lengthAxis) * normalize(axis));
}