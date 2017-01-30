#version 150 core

in vec4 pass_Color;
in vec3 pass_Uvs;

out vec4 out_Color;

uniform sampler2D	myTextureSampler;
uniform vec4		mColor;

void main(void)
{
	vec2 uvs = vec2(pass_Uvs.x, pass_Uvs.y);

	// Fetch both colors
	vec3  color_one = texture(myTextureSampler, uvs).brg;

	// Set output color
	out_Color = vec4(color_one, 1.0) * mColor;
}