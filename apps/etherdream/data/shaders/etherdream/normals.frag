#version 330

// input  
in vec4 pass_Color;

// output
out vec4 out_Color;

// Uniform used for setting color
uniform vec4 mColor;

void main() 
{
	vec3 line_color = mColor.rgb * pass_Color.rgb;
	out_Color = vec4(line_color, pass_Color.a * mColor.a);
}
