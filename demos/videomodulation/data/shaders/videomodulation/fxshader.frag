#version 330

// vertex shader input  
in vec3 passUVs;						//< frag Uv's
in vec3 passPosition;					//< frag world space position 

// uniform inputs
uniform sampler2D	inputTexture;		//< input texture
uniform float		intensity;			//< greyscale intensity value

// output
out vec4 out_Color;

void main() 
{
	// Get texture rgb value
	vec3 tex_color = texture(inputTexture, passUVs.xy).rgb;
	float greyscale = ((tex_color.r + tex_color.g + tex_color.b) / 3.0 * intensity); 
	out_Color =  vec4(greyscale, greyscale, greyscale, 1.0);
}