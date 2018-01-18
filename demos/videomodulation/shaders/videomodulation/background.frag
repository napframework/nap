#version 330

// vertex shader input  
in vec3 passUVs;						//< frag Uv's
in vec3 passPosition;					//< frag world space position 

// uniform inputs
uniform sampler2D	videoTexture;
uniform vec3		colorOne;
uniform vec3		colorTwo;

// output
out vec4 out_Color;

void main() 
{
	// Get texture rgba value
	vec3 tex_color = texture(videoTexture, passUVs.xy).rgb;

	float greyscale = (tex_color.r + tex_color.g + tex_color.b) / 3.0; 
	vec3 out_color = mix(colorOne, colorTwo, greyscale);

	// Set fragment color output to be texture color
	out_Color =  vec4(out_color, 1.0);
}