#version 330

// vertex shader input  
in vec3 passUVs;						//< frag Uv's
in vec3 passPosition;					//< frag world space position 

// uniforms
uniform sampler2D cloudTexture;			//< Rendered cloud texture
uniform sampler2D videoTexture;			//< Rendered video texture
uniform float blendValue;				//< Cloud to video blend value

// output
out vec4 out_Color;

void main() 
{
	// initial texture coordinate
	vec3 cloud_color = texture(cloudTexture, passUVs.xy).rgb;
	vec3 video_color = texture(videoTexture, passUVs.xy).rgb;

	// set fragment color
	out_Color =  vec4(mix(cloud_color, video_color, blendValue), 1.0);
}