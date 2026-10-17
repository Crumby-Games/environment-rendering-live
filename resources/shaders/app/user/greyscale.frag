#version 460 core

layout (location = 0) in vec2 vUV;

layout (location = 0) out vec4 oColor;

uniform sampler2D uColor;

void main()
{
	vec3 color = texture(uColor, vUV).rgb;

	// Weigh R, G and B channels differently based on the sRGB standard.
	float grey = dot(color, vec3(0.2126, 0.7152, 0.0722));

	oColor = vec4(vec3(grey), 1.0);
}
