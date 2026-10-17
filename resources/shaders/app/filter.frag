// Sections of this script is not my own. Functions that are directly based on external sources are clearly tagged.

#version 460 core

layout (location = 0) out vec4 oColor;
layout (location = 0) in vec2 vUV;

uniform sampler2D uEnvironment;
uniform float uRoughness = 1.0;
uniform float uLod = 0.0;

const int SAMPLE_COUNT = 64;
const float PI = 3.14159265359;

// Minor optimisation by precomputing at compile-time. Mathematically, x = 1 / (2pi), y = 1 / pi
const vec2 INV_ATAN = vec2(0.1591, 0.3183);

// Converts a point on a spherical map (based on direction from origin) into a 2D UV coordinate on the environment image
// (Equirectangular mapping)
vec2 dirToUV(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= INV_ATAN;
    uv += 0.5;
    return uv;
}

// Converts a point on a equirectangular map (2D UV coordinate) into a direction for sampling a spherical map
vec3 uvToDir(vec2 uv)
{
    uv -= 0.5;
    uv /= INV_ATAN;
    float r = cos(uv.y);
    float x = r * cos(uv.x);
    float y = sin(uv.y);
    float z = r * sin(uv.x);

    return vec3(x, y, z);
}

// Generate a pseudo-random 1D sample scalar
// Bit-wise tricks to get an evenly distributed scalar sample using the Van der Corput sequence
// [LearnOpenGL] https://learnopengl.com/PBR/IBL/Specular-IBL
float radicalInverse_VdC(uint bits)
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10;
}

// Generate a pseudo-random 2D sample coordinate
// [LearnOpenGL] https://learnopengl.com/PBR/IBL/Specular-IBL
vec2 hammersley(uint i, uint N)
{
    return vec2(float(i) / float(N), radicalInverse_VdC(i));
}

// Generate a pseudo-random 3D sample direction
// Instead of sampling directions uniformly, samples are biased towards directions that contribute more light.
// i.e. When roughness is low, more samples are near N. When roughness is high, samples are more spread out from N.
// [LearnOpenGL] https://learnopengl.com/PBR/IBL/Specular-IBL
vec3 importanceSampleGGX(vec2 Xi, vec3 N, float roughness)
{
    float a = roughness*roughness;
    float phi = 2.0 * PI * Xi.x;
    float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a*a - 1.0) * Xi.y));
    float sinTheta = sqrt(1.0 - cosTheta*cosTheta);
	
    // from spherical coordinates to cartesian coordinates
    vec3 H;
    H.x = cos(phi) * sinTheta;
    H.y = sin(phi) * sinTheta;
    H.z = cosTheta;
	
    // from tangent-space vector to world-space sample vector
    vec3 up        = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangent   = normalize(cross(up, N));
    vec3 bitangent = cross(N, tangent);
	
    vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
    return normalize(sampleVec);
} 

vec3 getSampleDir(vec3 dir, uint idx) {
    vec2 Xi = hammersley(idx, uint(SAMPLE_COUNT));
    vec3 H = importanceSampleGGX(Xi, dir, uRoughness);
    return reflect(-dir, H);
}

void main()
{
    vec3 dir = normalize(uvToDir(vUV));

    vec3 prefiltered = vec3(0.0);
    float totalWeight = 0.0;

    // Sum samples together
    for (int i = 0; i < SAMPLE_COUNT; i++)
    {
        vec3 sampleDir = getSampleDir(dir, i);

        // Only take sample if it is in the correct hemisphere
        float weight = max(dot(dir, sampleDir), 0.0);
        vec2 uv = dirToUV(sampleDir);
        prefiltered += textureLod(uEnvironment, uv, uLod).rgb * weight;
        totalWeight += weight;
    }

    // Find the average of the samples to perform a blur
    prefiltered /= max(totalWeight, 0.0001);

    oColor = vec4(prefiltered, 1.0);
}
