#version 330

// vertex shader input  
in vec3 passUVs;						//< frag Uv's
in vec3 passPosition;					//< frag world space position 

// uniforms
uniform sampler2D uTextureOne;			//< First texture
uniform sampler2D uTextureTwo;			//< Second texture
uniform float uBlendValue;				//< Blend between two textures

// output
out vec4 out_Color;

void main() 
{
	// initial texture coordinate
	vec3 colorOne = texture(uTextureOne, passUVs.xy).rgb;
	vec3 colorTwo = texture(uTextureTwo, passUVs.xy).rgb;

	// set fragment color
	out_Color =  vec4(mix(colorOne, colorTwo, uBlendValue), 1.0);
}
