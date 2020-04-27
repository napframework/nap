#version 450 core

in vec4 pass_Color;
in vec3 pass_Uvs;

out vec4 out_Color;

void main(void)
{
	// Set output color
	out_Color = pass_Color;
}