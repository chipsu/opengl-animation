#version 450

layout(location=0) in vec3 inColor;
layout(location=1) in vec3 inNormal;
layout(location=2) in vec3 inPosition;
layout(location=3) in vec3 inBarycentric;

layout(location=0) out vec4 outColor;

uniform vec3 uLightPos; 
uniform vec3 uViewPos; 
uniform vec3 uLightColor;

void main() {
    outColor = vec4(inColor, 1.0);

	if(inBarycentric.x < 0.01 || inBarycentric.y < 0.01 || inBarycentric.z < 0.01) {
	    outColor = outColor * 0.25;
	}
} 