#version 460 core

layout (location = 0) in vec2 vUV;

layout (location = 0) out vec4 oColor;

uniform sampler2D uColor;
uniform sampler2D gNormal;
uniform sampler2D gDepth;

void main()
{
	vec3 normal = texture(gNormal, vUV).xyz;
	vec3 color = normal * 0.5 + 0.5;

	vec3 scene = texture(uColor, vUV).rgb;
	float depth = texture(gDepth, vUV).r;
	oColor = vec4(mix(color, scene, int(depth)), 1.0);
}
