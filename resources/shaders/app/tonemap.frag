#version 460 core

layout (location = 0) in vec2 vUV;

layout (location = 0) out vec4 oColor;

uniform sampler2D uColor;
uniform int uAlgorithmMode = 0; // 0 Reinhard, 1 ACES-approximation
uniform float uExposure = 1.0f;

// Approximation of the ACES tone mapping curve
// [Kryzstof Narkowicz] https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/
vec3 ACESFilm(vec3 x)
{
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;

    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

void main() {
	vec3 hdr = texture(uColor, vUV).rgb * uExposure;

	vec3 mapped;

	// Reinhard tone mapping. Regardless of exposure, result always has every channel at < 1.0.
	// The higher the initial values are, the more compressed the result becomes towards 1.
	// In other words, high values results in a brighter, less saturated result.
	// Regardless of exposure, the output will always be between 0 and 1.
	if (uAlgorithmMode == 0) mapped = hdr / (hdr + vec3(1.0));

	// An approximation of tone mapping used in ACES.
	// Unlike Reinhard, clamping will happen if exposure is too high.
	if (uAlgorithmMode == 1) mapped = ACESFilm(hdr);

	oColor = vec4(mapped, 1.0);
}
