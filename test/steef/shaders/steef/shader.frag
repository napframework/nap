#version 150 core

in vec4 pass_Color;
in vec3 pass_Uvs0;	// The global vinyl uvs
in vec3 pass_Uvs1;	// The label uvs

out vec4 out_Color;

uniform sampler2D	vinylLabel;
uniform sampler2D	vinylMask;
uniform vec4		mColor;

void main(void)
{
	// Get color of mask based on first uv set
	float mask_color = texture(vinylMask, pass_Uvs0.xy).r;
	
	// Based on that we fetch the right uv set, values > 0 are not the label
	vec3 color = vec3(0.0, 0.0, 0.0);
	if(mask_color > 0.01)
	{
		vec2 uvs = vec2(pass_Uvs1.x, pass_Uvs1.y);
		color = texture(vinylLabel, uvs).rgb;
	}

	// Fetch both colors
	// vec3 color = texture(vinylLabel, uvs).rgb;

	// Set output color
	out_Color = vec4(color, 1.0) * mColor;
}