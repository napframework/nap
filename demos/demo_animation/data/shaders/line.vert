#version 330 core

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

in vec3	in_Position;
in vec3	in_UV0;
in vec4 in_Color0;
in vec3 in_Normals;

out vec3 pass_Uvs;
out vec4 pass_Color;
out vec3 pass_Normals;

void main(void)
{
	// Calculate position
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(in_Position, 1.0);

	// Pass uvs 
	pass_Uvs = in_UV0;

	// Pass color
	pass_Color = in_Color0;

	// Pass normals
	pass_Normals = in_Normals;
}