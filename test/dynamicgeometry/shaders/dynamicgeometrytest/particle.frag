#version 150 core

in vec4 pass_Color;
in vec3 pass_Uvs;

out vec4 out_Color;

uniform sampler2D	particleTextureOne;


void main(void)
{
	vec2 uvs = vec2(pass_Uvs.x, pass_Uvs.y);
	vec4 tex_color = texture(particleTextureOne, uvs);

	// Set output color
	out_Color = tex_color * pass_Color;
}