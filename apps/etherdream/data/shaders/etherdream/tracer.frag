#version 330

// input
in vec3 pass_Normals;

// output
out vec4 out_Color;

// uniforms
uniform vec3 mColor;

void main() 
{
	float n_scale = pass_Normals.z;
	vec3 mix_color = vec3(1.0,1.0,1.0);
	vec3 color = mix(mix_color, mColor, pow(n_scale,1.5));
	out_Color =  vec4(color , 1.0); 
}

