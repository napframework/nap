#version 330

// vertex shader input  
in vec3 passUVs;						//< frag Uv's
in vec3 passPosition;					//< frag world space position 

// uniforms
uniform sampler2D combinationTexture;	//< Rendered combination texture

// output
out vec4 out_Color;

void main() 
{
	// initial texture coordinate
	vec3 color = texture(combinationTexture, passUVs.xy).rgb;

	// set fragment color
	out_Color =  vec4(color, 1.0);
}