#version 460 core

layout (location = 0) in vec2 vUV;

layout (location = 0) out vec4 oColor;

uniform sampler2D uColor;
uniform sampler2D gDepth;
uniform float uTime;
uniform float uSpeed = 3.0;
uniform float uFrequency = 10.0;
uniform float uWidth = 0.01;

void main() {
	vec2 offsetUV = vUV;
	offsetUV.x += cos((uTime*uSpeed)+vUV.y*uFrequency*2.0)*uWidth;
	vec4 color = texture(uColor,offsetUV);

	vec4 scene = texture(uColor,vUV);
	float depth = texture(gDepth,vUV).r;

	// Wave-aware depth mask
	if (depth == 1.0) depth = texture(gDepth, offsetUV).r;

	oColor = mix(color, scene, int(depth));
}
