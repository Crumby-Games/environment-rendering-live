#version 460 core

layout (location = 0) out vec4 oColor;
layout (location = 0) in vec2 vUV;

uniform sampler2D uEnvironment;
uniform float uLod = 0;

const int SAMPLE_COUNT = 512;
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

// Cosine-weighted hemisphere sampling around normal
vec3 sampleHemisphere(vec3 normal, vec2 xi)
{
    float phi = 2.0 * PI * xi.x;
    float cosTheta = sqrt(1.0 - xi.y);
    float sinTheta = sqrt(xi.y);

    vec3 tangent = normalize(abs(normal.y) < 0.999 ? cross(normal, vec3(0,1,0)) : cross(normal, vec3(1,0,0)));
    vec3 bitangent = cross(normal, tangent);

    vec3 sampleDir =
        tangent * cos(phi) * sinTheta +
        bitangent * sin(phi) * sinTheta +
        normal * cosTheta;

    return normalize(sampleDir);
}

void main()
{
    vec3 normal = normalize(uvToDir(vUV));

    vec3 irradiance = vec3(0.0);
    float weight = 0.0;

    // Sum radiance together
    for (int i = 0; i < SAMPLE_COUNT; i++)
    {
        // Pseudo-random sampling coordinate
        vec2 xi = vec2(
            fract(sin(float(i) * 12.9898) * 43758.5453),
            fract(sin(float(i) * 78.233) * 43758.5453)
        );

        vec3 sampleDir = sampleHemisphere(normal, xi);
        vec3 radiance = textureLod(uEnvironment, dirToUV(sampleDir), uLod).rgb;

        // Only include values in the relevant hemisphere
        float nDotL = max(dot(sampleDir, normal), 0.0); // i.e. cos(theta).

        irradiance += radiance * nDotL;
        weight += nDotL;
    }

    // Since radiance is cosine-weighted, the sum must be divided by the total cos(theta) weight
    irradiance = irradiance / max(weight, 0.0001);

    oColor = vec4(irradiance, 1.0);
}