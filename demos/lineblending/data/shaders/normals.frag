#version 450 core

// input  
in float pass_Tip;
in vec4 pass_Color;

// output
out vec4 out_Color;

// Uniform used for setting color
uniform UBO
{
	uniform vec4 mColor;
} ubo;

void main() 
{
	vec3 line_color = ubo.mColor.rgb;
	out_Color = vec4(line_color * pass_Color.rgb, pass_Tip * ubo.mColor.a);
}
