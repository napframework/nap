#version 330

// input  
in vec4 pass_Color;

// output
out vec4 out_Color;

void main() 
{
	vec3 color = pass_Color.rgb * pass_Color.a;
	out_Color = vec4(color, 1.0);
}
