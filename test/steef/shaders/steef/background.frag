#version 150 core
in vec3 pass_Uvs;

out vec4 out_Color;

uniform sampler2D	backgroundTexture;
uniform float		intensity;

void main(void)
{
	// Get color from texture
	vec3 color = texture(backgroundTexture, pass_Uvs.xy).rgb;

	// Set output color
	out_Color = vec4(color * intensity, 1.0);
}