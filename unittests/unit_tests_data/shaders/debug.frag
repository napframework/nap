#version 150 core

in vec4 pass_Color;
in vec3 pass_Uvs;

out vec4 out_Color;

uniform sampler2D	debugImage;			// Image to display

void main(void)
{
	// Fetch cover image color
	vec3 color = texture(debugImage, pass_Uvs.xy).rgb;

	// Set output color
	out_Color = vec4(color, 1.0) * pass_Color;
}