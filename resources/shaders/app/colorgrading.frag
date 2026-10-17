#version 460 core

layout (location = 0) in vec2 vUV;

layout (location = 0) out vec4 oColor;

uniform sampler2D uColor;
uniform float uBrightness;
uniform float uContrast;
uniform float uSaturation;
uniform float uExposure;
uniform vec3 uTint;

const vec3 LUMINANCE_RATIOS = vec3(0.2126, 0.7152, 0.0722);


void main() {
    vec3 color = texture(uColor, vUV).rgb;
    
    color *= uExposure;
    color = color + uBrightness;
    color = 0.5 + uContrast * (color - 0.5);
    float luminance = dot(color, LUMINANCE_RATIOS);
    color = mix(vec3(luminance), color, uSaturation);
    color = uTint * color;
    color = clamp(color,0.0,1.0);
    oColor = vec4(color, 1.0);
}
