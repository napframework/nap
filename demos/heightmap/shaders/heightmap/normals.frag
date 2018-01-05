#version 330

// input  
in vec4 pass_Color;

// output
out vec4 out_Color;

// Uniform used for setting color
uniform vec4 color;
uniform float length;

void main() 
{
	float v = 1.0-clamp(pass_Color.a, 0.0, 1.0);
	float m = length;
	v =  1.0 - (v / (m) * 1.0);
	out_Color = vec4(color.rgb * pass_Color.rgb, color.a * v);
}
