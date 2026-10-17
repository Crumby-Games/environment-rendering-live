#version 460 core

layout (location = 0) out vec4 oColor;

layout (location = 0) in vec2 vUV;

uniform sampler2D uDepth;
uniform vec3 uNearColor = vec3(1.0);
uniform vec3 uFarColor = vec3(0.0);
uniform float uNear;
uniform float uFar;

float LineariseDepth(float z)
{
	float ndc = z * 2.0 - 1.0;
	float viewZ = (2.0 * uNear * uFar) / (uFar + uNear - ndc * (uFar - uNear));
	return (viewZ - uNear) / (uFar - uNear);
}	

void main()
{
	float z = texture(uDepth, vUV).r;
	if (z == 1.0) discard;
	float tGamma = pow(LineariseDepth(z), 1.0 / 1.7);
	vec3 color = mix(uNearColor, uFarColor, tGamma);
	oColor = vec4(color, 1.0);
}
