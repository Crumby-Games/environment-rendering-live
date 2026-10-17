#version 460 core

layout (location = 0) in vec2 vUV;
layout (location = 0) out vec4 oColor;

uniform sampler2D uColor;
uniform sampler2D gDepth;
uniform sampler2D gNormal;
uniform sampler2D uNoise;

uniform int uKernelSize; // Maximum kernel size is 64
uniform vec3 uKernel[64];
uniform vec2 uNoiseScale;
uniform mat4 uProj;
uniform mat4 uInvProj;

uniform float uIntensity;
uniform float uRadius;

const float BIAS = 0.01f;

vec3 reconstructViewPos(vec2 uv, float depth)
{
    vec4 clip = vec4(uv * 2.0 - 1.0, depth, 1.0);
    vec4 view = uInvProj * clip;
    return view.xyz / view.w;
}

mat3 buildTBN(vec3 normal, vec3 randomVec)
{
    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    return mat3(tangent, bitangent, normal);
}

void main()
{
    float depth = texture(gDepth, vUV).r;

    if (depth >= 1.0)
    {
        oColor = vec4(0.0);
        return;
    }

    vec3 normal = texture(gNormal, vUV).xyz * 2.0 - 1.0;

    // Get view-space position of the fragment
    vec3 position = reconstructViewPos(vUV, depth); 

    // Randomisation to prevent visual banding
    vec3 randomVec = texture(uNoise, vUV * uNoiseScale).xyz * 2.0 - 1.0; 
    mat3 TBN = buildTBN(normal, randomVec);

    float visibility = 0.0;

    // Sum visibility samples together
    for (int i = 0; i < uKernelSize; i++)
    {
        // Get a random position in fixed distance in the hemisphere from the fragment position
        vec3 samplePos = position + TBN * uKernel[i] * uRadius;

        // Get 2D coordinate of whichever point would be collided with in that direction
        vec4 clip = uProj * vec4(samplePos, 1.0);
        vec3 ndc = clip.xyz / clip.w;
        vec2 sampleUV = ndc.xy * 0.5 + 0.5;

        // Get 3D coordinate of the potentially occluding point
        float occludeDepth = texture(gDepth, sampleUV).r;
        vec3 occludePos = reconstructViewPos(sampleUV, occludeDepth);

        // Check whether occluding point is in range
        float dist = abs(position.z - occludePos.z);
        float range = smoothstep(0.0, 1.0, uRadius / dist); 

        // Add visiblity weighted by how far from the fragment the occlusion is
        visibility += (occludePos.z < samplePos.z + BIAS ? 0.0 : 1.0) * range;
    }

    float occlusion = 1.0 - (visibility / float(uKernelSize));
    occlusion = mix(1.0, occlusion, uIntensity);
    vec3 color = texture(uColor, vUV).rgb;
    oColor = vec4(color * occlusion, 1.0);
}
