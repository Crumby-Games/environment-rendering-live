#version 460 core

layout (location = 0) in vec3 aPos;

layout (location = 0) out vec3 vPos;

uniform mat4 uView;
uniform mat4 uProj;

void main()
{
    vPos = aPos;

    // Discard view translation since the skybox should remain visually static when panning or moving the camera.
    mat4 viewRotation = mat4(mat3(uView)); 

    vec4 pos = (uProj * viewRotation) * vec4(aPos, 1.0);

    // Replaces z (depth) with w, which will always be 1.0.
    // Therefore, depth will always read as 1.0, making skybox render behind everything else when depth testing.
    // (Swizzling)
    gl_Position = pos.xyww;
}
