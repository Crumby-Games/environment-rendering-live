#version 460 core

layout (location = 0) in vec2 vUV;

layout (location = 0) out vec4 oColor;

uniform sampler2D uColor;

const float RADIUS = 0.75;
const float SOFTNESS = 0.45;
const float OPACITY = 0.5;

void main() {
    vec4 color = texture2D(uColor, vUV);
    vec2 position = (vUV) - vec2(0.5);
    float len = length(position);
    float vignette = smoothstep(RADIUS, RADIUS - SOFTNESS, len);
    vec3 finalColor = mix(color.rgb, color.rgb * vignette, OPACITY);
    
    oColor = vec4(finalColor, color.a);
}
