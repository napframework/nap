#version 150 core

in vec4 pass_Color;
in vec3 pass_Uvs;

out vec4 out_Color;

uniform sampler2D	backgroundTexture;

void main(void)
{
	// Get color from texture
	vec3 color = texture(backgroundTexture, pass_Uvs.xy).rgb;

	// Set output color
	out_Color = vec4(color * 0.6, 1.0);
}