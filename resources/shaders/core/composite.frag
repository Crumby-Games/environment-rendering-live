#version 460 core

layout (location = 0) in vec2 vUV;

layout (location = 0) out vec4 oColor;

uniform sampler2D uBackColor;
uniform sampler2D uFrontColor;

void main()
{
	vec4 bg = texture(uBackColor, vUV);
	vec4 fg = texture(uFrontColor, vUV);

	oColor = vec4(mix(bg, fg, fg.a));
}
