#version 450

uniform sampler2D colorTexture;		// The input color texture to sample from

in vec2 pass_UVs[3];
out vec4 out_Color;

// Sampler must be configured with filter VK_FILTER_LINEAR
vec4 blur(sampler2D tx) 
{
	vec4 col = vec4(0.0);
	col += texture(tx, pass_UVs[0]) * 0.2941176471;
	col += texture(tx, pass_UVs[1]) * 0.3529411765;
	col += texture(tx, pass_UVs[2]) * 0.3529411765;
	return col;
}

void main() 
{
	out_Color = blur(colorTexture);
}
 