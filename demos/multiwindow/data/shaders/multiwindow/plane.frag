#version 330

// vertex shader input  
in vec3 passUVs;						//< frag Uv's
in vec3 passPosition;					//< frag world space position 

// uniform inputs
uniform sampler2D inTexture;

// output
out vec4 out_Color;

void main() 
{
	// Get texture rgba value
	vec4 tex_color = texture(inTexture, passUVs.xy).rgba;

	// Set fragment color output to be texture color
	out_Color =  vec4(tex_color);
}