#version 330

// vertex input  
in vec3 passUVs;

// uniform inputs
uniform sampler2D inWorldTexture;		//< World Texture

// output
out vec4 out_Color;

void main() 
{
	// Use alpha to blend between two colors
	float alpha = texture(inWorldTexture, passUVs.xy).a;
	vec3 world_color = mix(vec3(0.784, 0.411, 0.411), vec3(0.176, 0.180, 0.258), alpha);

	// Set fragment color output
	out_Color =  vec4(world_color,1.0);
}
