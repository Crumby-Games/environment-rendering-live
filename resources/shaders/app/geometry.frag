#version 460 core

layout (location = 0) in vec3 vNormal;

// Uses vec4 since framebuffer color attachments are RGBA, even though alpha channels are not utilised)
layout (location = 0) out vec4 oAlbedo;
layout (location = 1) out vec4 oNormal;

uniform vec3 uAlbedo;

void main()
{
	// Alpha channels could be used to store additional material properties in the future. Currently arbitrary.
	oAlbedo = vec4(uAlbedo, 1.0);
	oNormal = vec4(vNormal, 0.0);
}
