#version 150 core

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

in vec3	in_Position;		// Vertex Position
in vec3 in_Normals;			// Vertex Normals

out vec3 pass_Normals;
out vec3 pass_Vert;
out mat4 pass_ModelMatrix;

void main(void)
{
	// Calculate position
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(in_Position, 1.0);

	// Pass along vertex position (object space)
	pass_Vert = in_Position;

	// Pass along model matrix for light calculations
	pass_ModelMatrix = modelMatrix;

	// Pass along normals for light calculations
	pass_Normals = in_Normals;
}