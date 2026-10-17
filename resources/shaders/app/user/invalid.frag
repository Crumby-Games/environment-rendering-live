#version 460 core

layout (location = 0) in vec2 vUV;

layout (location = 0) out vec4 oColor;

uniform sampler2D uColor;

void main()
{
	vec3 color = texture(DOESNOTEXIST, vUV).rgb;

	oColor = vec4(color, 1.0; // no bracket
}
