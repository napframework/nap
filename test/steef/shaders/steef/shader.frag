#version 150 core

in vec4 pass_Color;
in vec3 pass_Uvs0;	// The global vinyl uvs
in vec3 pass_Uvs1;	// The label uvs

out vec4 out_Color;

uniform sampler2D	vinylLabel;
uniform sampler2D	vinylMask;
uniform vec3		recordColor;

void main(void)
{
	// Get color of mask based on first uv set
	float mask_color = texture(vinylMask, pass_Uvs0.xy).r;

	// Label color
	vec3 label_color = texture(vinylLabel, pass_Uvs1.xy).rgb;

	// Blend based on mask color
	vec3 out_color = mix(recordColor, label_color, mask_color);

	// Set output color
	out_Color = vec4(out_color, 1.0);
}