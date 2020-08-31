#version 450 core

// input  
in float pass_Tip;

// output
out vec4 out_Color;

// All fragment shader ubo's
uniform FRAGUBO
{
	uniform vec3 color;				//< Normal color
	uniform float opacity;			//< Opacity
	uniform float nlength;			//< Normal length
} fubo;

void main() 
{
	float v = 1.0-clamp(pass_Tip, 0.0, 1.0);
	float m = fubo.nlength;
	v =  1.0 - (v / (m) * 1.0);
	out_Color = vec4(fubo.color.rgb, fubo.opacity * v);
}