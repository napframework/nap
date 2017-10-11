#version 150 core

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

in vec3		in_Position;
in vec4		in_Color;
in vec3		in_Uvs;

out vec4 pass_Color;
out vec3 pass_Uvs;

void main(void)
{
	// Calculate position
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(in_Position, 1.0);

	// Pass color and uv's 
	pass_Color = in_Color;
	pass_Uvs = in_Uvs;
}