#version 450 core

layout(location=0) in vec4 pass_Color;
layout(location=1) in vec3 pass_Uvs;

layout(location=0) out vec4 out_Color;

void main(void)
{
	// Set output color
	out_Color = pass_Color;
}