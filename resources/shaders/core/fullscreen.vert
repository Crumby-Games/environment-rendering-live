#version 460 core

layout (location = 0) in vec3 aPos;

layout (location = 0) out vec2 vUV;

void main()
{
	// Map the (-1, -1), (1, 1) coordinate range of the fullscreen renderer to (0,0), (1,1)
	vUV = aPos.xy * 0.5 + 0.5;
	gl_Position = vec4(aPos, 1.0);
}
