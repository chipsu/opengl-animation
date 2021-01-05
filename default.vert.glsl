#version 450

#define MAX_BONES 1000

layout(location=0) uniform mat4 uProj;
layout(location=1) uniform mat4 uView;
layout(location=2) uniform mat4 uModel;
layout(location=3) uniform mat4 uBones[MAX_BONES];

layout(location=0) in vec3 inPosition;
layout(location=1) in vec3 inNormal;
layout(location=2) in vec3 inColor;
layout(location=3) in vec4 inBoneWeights;
layout(location=4) in uvec4 inBoneIndices;

layout(location=0) out vec3 outColor;

void main() {
    mat4 boneTransform = uBones[inBoneIndices[0]] * inBoneWeights[0];
    boneTransform += uBones[inBoneIndices[1]] * inBoneWeights[1];
    boneTransform += uBones[inBoneIndices[2]] * inBoneWeights[2];
    boneTransform += uBones[inBoneIndices[3]] * inBoneWeights[3];
    
    gl_Position = uProj * uView * uModel * boneTransform * vec4(inPosition, 1.0);
    //gl_Position = uProj * uView * uModel * vec4(inPosition, 1.0);

    outColor = inColor;
}
