#version 450 core

in vec3 pass_Uvs;
out vec4 out_Color;

// Uniform buffer inputs
uniform UBO
{
	uniform float intensity;
} ubo;

// Samplers
uniform sampler2D	backgroundTexture;

void main(void)
{
	// Get color from texture
	vec3 color = texture(backgroundTexture, pass_Uvs.xy).rgb;

	// Set output color
	out_Color = vec4(color * ubo.intensity, 1.0);
}