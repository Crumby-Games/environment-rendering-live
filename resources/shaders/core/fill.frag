#version 460 core

layout (location = 0) out vec4 oColor;

uniform vec3 uFillColor = vec3(0);

void main()
{
	oColor = vec4(uFillColor,1.0);
}
