#version 150 core

in vec4 pass_Color;
in vec3 pass_Uvs;

out vec4 out_Color;

uniform sampler2D	imageA;			// Image to display
uniform sampler2D	imageB;			// Image to display

void main(void)
{
	// Fetch cover image color
	vec3 color_a = texture(imageA, pass_Uvs.xy).rgb;
	vec3 color_b = texture(imageB, pass_Uvs.xy).rgb;

	vec3 color = color_a;
	float greyscale = (color_b.r + color_b.g + color_b.b) / 3.0;
	if(greyscale > 0.01)
	{
		color = color_b;
	} 

	// Set output color
	out_Color = vec4(color, 1.0) * pass_Color;
}