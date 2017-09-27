#version 150 core

in vec3 pass_Uvs;
in vec4 pass_Color;

out vec4 out_Color;

void main(void)
{
	//out_Color = vec4(pass_Color.r * pass_Uvs.r, pass_Color.g * pass_Uvs.g, 0.0f, pass_Color.a);
	out_Color = pass_Color;
}