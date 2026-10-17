#version 460 core

layout (location = 0) in vec2 vUV;

layout (location = 0) out vec4 oColor;

void main()
{
	oColor = vec4(vUV, 0.0, 1.0);
}
