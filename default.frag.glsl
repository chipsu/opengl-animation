#version 450

layout(location=0) in vec3 inColor;
layout(location=1) in vec3 inNormal;
layout(location=2) in vec3 inPosition;

layout(location=0) out vec4 outColor;

uniform vec3 uLightPos;
uniform vec3 uViewPos;
uniform vec3 uLightColor;

uniform float uAmbientStrength = 0.2;
uniform float uDiffuseStrength = 1.0;
uniform float uSpecularStrength = 0.1;

void main() {
    vec3 norm = normalize(inNormal);
    vec3 lightDir = normalize(uLightPos - inPosition);
    vec3 viewDir = normalize(uViewPos - inPosition);
    vec3 reflectDir = reflect(-lightDir, norm);
    vec3 ambient = uAmbientStrength * uLightColor;

    float diffuseAmount = max(dot(norm, lightDir), 0.1);
    vec3 diffuse = uDiffuseStrength * diffuseAmount * uLightColor;

    float specularAmount = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = uSpecularStrength * specularAmount * uLightColor;

    vec3 rgb = (ambient + diffuse + specular) * inColor;
    outColor = vec4(rgb, 1.0);
}
