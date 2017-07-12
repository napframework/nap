#version 150 core

in vec4 pass_Color;
in vec3 pass_Uvs0;	// The global cover uvs

out vec4 out_Color;

uniform sampler2D	coverImage;

void main(void)
{
	// Fetch both colors
	vec3 color = texture(coverImage, pass_Uvs0.xy).rgb;

	// Set output color
	out_Color = vec4(color, 1.0);
}