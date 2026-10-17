#version 460 core

layout (location = 0) in vec2 vUV;

layout (location = 0) out vec4 oColor;

uniform sampler2D uColor;
uniform float uGamma = 2.2f;

void main() {
	vec3 color = texture(uColor, vUV).rgb;

	// Produces logarithmic luminance to better suit human perception of brightness
	// Common approximation of the sRGB standard
	oColor = vec4(pow(color, vec3(1.0 / uGamma)), 1.0);
}
