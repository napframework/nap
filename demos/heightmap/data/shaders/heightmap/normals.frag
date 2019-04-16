#version 330

// input  
in float pass_Tip;

// output
out vec4 out_Color;

// Uniform used for setting color
uniform vec3 color;				//< Normal color
uniform float opacity;			//< Opacity
uniform float length;			//< Normal length

void main() 
{
	float v = 1.0-clamp(pass_Tip, 0.0, 1.0);
	float m = length;
	v =  1.0 - (v / (m) * 1.0);
	out_Color = vec4(color.rgb, opacity * v);
}
