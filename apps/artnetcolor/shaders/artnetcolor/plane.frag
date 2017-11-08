#version 330

// input  
in vec3 pass_Uvs;
in vec4 pass_Color;

// output
out vec4 out_Color;

// uniforms
uniform vec3	mColor;
uniform float	mWhite;

void main() 
{
	float r = clamp(mColor.r + mWhite,0.0,1.0);
	float g = clamp(mColor.g + mWhite,0.0,1.0);
	float b = clamp(mColor.b + mWhite,0.0,1.0);

	vec4 final_color = vec4(r,g,b,1.0);
	out_Color =  final_color;
}
