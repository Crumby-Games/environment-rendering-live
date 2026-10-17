#version 460 core

layout(location = 0) in vec2 vUV;
layout(location = 0) out vec4 oColor;

uniform sampler2D gAlbedo;
uniform sampler2D gNormal;
uniform sampler2D gDepth;
uniform sampler2D uEnvironment;
uniform sampler2D uPrefiltered;
uniform sampler2D uIrradiance;

const int MODE_LAMBERTIAN = 1;
const int MODE_IBL = 2;
const int MODE_ENVMAP = 4;
const int MODE_FRESNEL_SPLIT = 8;

uniform mat4 uInvProj;
uniform mat4 uInvView;
uniform int uModeFlags = MODE_LAMBERTIAN | MODE_IBL | MODE_ENVMAP | MODE_FRESNEL_SPLIT;
uniform float uEnvMappingWeight;
uniform float uRoughness;
uniform vec3 uSpecularColor;

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

vec3 fresnelSchlick(float cosTheta, vec3 f0)
{
    return f0 + (1.0 - f0) * pow(1.0 - cosTheta, 5.0);
}

void main()
{
    float depth = texture(gDepth, vUV).r;
    if (depth >= 1.0)
    {
        oColor = vec4(0.0);
        return;
    }

    vec3 albedo = texture(gAlbedo, vUV).rgb;
    vec3 normal = texture(gNormal, vUV).xyz;
    normal = normalize(uInvView * vec4(normal, 0.0)).xyz;
    vec2 envUV = dirToUV(normal);

    vec3 worldPos = reconstructWorldPosition(vUV, depth);
    vec3 cameraPos = (uInvView * vec4(0.0, 0.0, 0.0, 1.0)).xyz; // in world space
    vec3 viewDir = normalize(cameraPos - worldPos);
    vec3 R = reflect(-viewDir, normal);
    vec3 specDir = normalize(mix(R, normal, uRoughness * uRoughness));
    float NdotV = max(dot(normal, viewDir), 0.0);
    
    vec3 prefiltered;
    vec3 irradiance;
    vec3 env;

    // Only sample what's necessary
    if ((uModeFlags & MODE_LAMBERTIAN) != 0) irradiance = texture(uIrradiance, envUV).rgb;
    if ((uModeFlags & MODE_ENVMAP) != 0) env = textureLod(uEnvironment, dirToUV(R), 0.0).rgb;
    if ((uModeFlags & MODE_IBL) != 0) prefiltered = texture(uPrefiltered, dirToUV(specDir)).rgb;

    vec3 specular = vec3(0.0);
    if ((uModeFlags & MODE_ENVMAP) != 0) {
        if ((uModeFlags & MODE_IBL) != 0) specular = mix(prefiltered, env, uEnvMappingWeight);
        else specular = env;
    } 
    else if ((uModeFlags & MODE_IBL) != 0)
        specular = prefiltered;

    vec3 color;
    if ((uModeFlags & MODE_FRESNEL_SPLIT) != 0) {
        vec3 F;
        vec3 diffuse = albedo;
        F = fresnelSchlick(NdotV, uSpecularColor);
        if ((uModeFlags & MODE_LAMBERTIAN) != 0) {
            diffuse *= irradiance * INV_ATAN.y;
        }
        if ((uModeFlags & (MODE_ENVMAP | MODE_IBL)) != 0)
            color = diffuse * (1.0 - F) + specular * F;
        else color = diffuse;
    }
    else color = specular;

    oColor = vec4(color, 1.0);
}
