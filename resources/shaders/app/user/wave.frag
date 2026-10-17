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
	oColor = texture(uColor,offsetUV);
}
