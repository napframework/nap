#version 330

// input  
in vec3 pass_Uvs;
in vec4 pass_Color;
in vec3 pass_Normals;

// output
out vec4 out_Color;

void main() 
{
	out_Color = pass_Color;
}
