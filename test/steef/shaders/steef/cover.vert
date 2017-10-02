#version 150 core

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

in vec3	in_Position;
in vec4	in_Color0;
in vec3 in_Normals;
in vec3	in_UV0;

out vec3 pass_Uvs0;
out mat4 pass_ModelMatrix;
out vec3 pass_Vert;
out vec3 pass_Normals;

void main(void)
{
	// Calculate position
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(in_Position, 1.0);

	// Pass color and uv's 
	pass_Uvs0 = in_UV0;
	pass_Vert = in_Position;
	pass_ModelMatrix = modelMatrix;
	pass_Normals = in_Normals;
}