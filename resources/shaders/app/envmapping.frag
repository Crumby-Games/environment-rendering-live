#version 460 core

layout(location = 0) in vec2 vUV;
layout(location = 0) out vec4 oColor;

uniform sampler2D uColor;
uniform sampler2D gNormal;
uniform sampler2D gDepth;
uniform sampler2D uEnvironment;

uniform mat4 uInvProj;
uniform mat4 uInvView;
uniform float uWeight;
uniform float uLod = 0.0;

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

vec3 reconstructWorldPosition(vec2 uv, float depth)
{
    vec4 clip = vec4(uv * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
    vec4 viewPos = uInvProj * clip;
    viewPos /= viewPos.w;
    vec4 worldPos = uInvView * viewPos;
    return worldPos.xyz;
}

void main()
{
    float depth = texture(gDepth, vUV).r;
    if (depth >= 1.0)
    {
        oColor = vec4(0.0);
        return;
    }

    vec3 normal = texture(gNormal, vUV).xyz;
    normal = normalize(uInvView * vec4(normal, 0.0)).xyz;
    vec3 worldPos = reconstructWorldPosition(vUV, depth);
    vec3 cameraPos = (uInvView * vec4(0.0, 0.0, 0.0, 1.0)).xyz; // in world space
    vec3 viewDir = normalize(worldPos - cameraPos);
    vec3 reflectionDir = reflect(viewDir, normal);

    vec4 color = texture(uColor, vUV);
    vec4 reflection = vec4(textureLod(uEnvironment, dirToUV(reflectionDir), uLod).rgb, 1.0);

    oColor = mix(color, reflection, uWeight);
}
