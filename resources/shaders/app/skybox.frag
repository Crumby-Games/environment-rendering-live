#version 460 core

layout (location = 0) in vec3 vPos;

layout (location = 0) out vec4 oColor;

uniform sampler2D uEnvironment;

// Minor optimisation by precomputing at compile-time.
const float PI = 3.14159265359;
const vec2 INV_ATAN = vec2(1.0 / (2.0 * PI), 1.0 / PI);

// Converts a point on a spherical map (based on direction from origin) into a 2D UV coordinate on the environment image
// (Equirectangular mapping)
vec2 dirToUV(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= INV_ATAN;
    uv += 0.5;
    return uv;
}

void main()
{
    vec3 dir = normalize(vPos);
    vec2 uv = dirToUV(dir);
    oColor = vec4(textureLod(uEnvironment, uv, 0.0).rgb, 1.0);
}
