#version 450

uniform sampler2D colorTexture;		// The input color texture to sample from

in vec2 pass_UVs[7];
out vec4 out_Color;

// Sampler must be configured with filter VK_FILTER_LINEAR
vec4 blur(sampler2D tx) 
{
	vec4 col = vec4(0.0);
	col += texture(tx, pass_UVs[0]) * 0.1964825502;
	col += texture(tx, pass_UVs[1]) * 0.2969069647;
	col += texture(tx, pass_UVs[2]) * 0.2969069647;
	col += texture(tx, pass_UVs[3]) * 0.0944703979;
	col += texture(tx, pass_UVs[4]) * 0.0944703979;
	col += texture(tx, pass_UVs[5]) * 0.0103813624;
	col += texture(tx, pass_UVs[6]) * 0.0103813624;

	return col;
}


void main() 
{
	out_Color = blur(colorTexture);
}
 