#version 450

layout(location=0) uniform mat4 uProj;
layout(location=1) uniform mat4 uView;
layout(location=2) uniform mat4 uModel;
layout(location=3) uniform float uTime;

layout(location=0) in vec3 inPosition;
layout(location=1) in vec3 inNormal;
layout(location=2) in vec3 inColor;

layout(location=0) out vec3 outColor;

void main() {
    float scale = inNormal.z;
    vec3 center = inPosition;
    mat4 viewModel = uView * uModel;
    vec3 camRight = vec3(viewModel[0][0], viewModel[1][0], viewModel[2][0]);
    vec3 camUp = vec3(viewModel[0][1], viewModel[1][1], viewModel[2][1]);
    vec3 pos = center
             + camRight * inNormal.x * scale
             + camUp * inNormal.y * scale;

    gl_Position = uProj * uView * uModel * vec4(pos, 1.0);

    outColor = inColor;
}
