#version 330

// input  
in float pass_Tip;
in vec4 pass_Color;

// output
out vec4 out_Color;

// Uniform used for setting color
uniform vec4 mColor;

void main() 
{
	vec3 line_color = mColor.rgb;
	out_Color = vec4(line_color * pass_Color.rgb, pow(pass_Tip,0.5) * mColor.a);
}
