#version 150 core

// Out color
out vec4 out_Color;

// Color of the frame
uniform vec3 mFrameColor;

void main(void)
{
	//out_Color = vec4(pass_Color.r * pass_Uvs.r, pass_Color.g * pass_Uvs.g, 0.0f, pass_Color.a);
	out_Color = vec4(mFrameColor,1.0);
}