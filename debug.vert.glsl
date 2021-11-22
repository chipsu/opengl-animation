#version 450

layout(location=0) uniform mat4 uProj;
layout(location=1) uniform mat4 uView;
layout(location=2) uniform mat4 uModel;

layout(location=0) in vec3 inPosition;
layout(location=2) in vec3 inColor;

layout(location=0) out vec3 outColor;

void main() {
    gl_Position = uProj * uView * uModel * vec4(inPosition, 1.0);
    outColor = inColor;
}
