#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

layout (location = 0) out vec3 vNormal;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProj;

void main()
{
	mat3 normalMatrix = transpose(inverse(mat3(uModel)));
    vec3 worldNormal = normalize(normalMatrix * aNormal);
	vNormal = mat3(uView) * worldNormal;

	gl_Position = uProj * uView * uModel * vec4(aPos, 1.0);
}
