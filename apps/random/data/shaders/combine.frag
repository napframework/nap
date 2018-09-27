#version 330

// vertex shader input  
in vec3 passUVs;						//< frag Uv's
in vec3 passPosition;					//< frag world space position 

// uniforms
uniform sampler2D cloudTexture;			//< Rendered cloud texture
uniform sampler2D sunTexture;			//< Rendered sun texture
uniform sampler2D videoTexture;			//< Rendered video texture
uniform float blendValue;				//< Cloud to video blend value

// output
out vec4 out_Color;

void main() 
{
	// initial texture coordinate
	vec3 cloud_color = texture(cloudTexture, passUVs.xy).rgb;
	vec3 sun_color = texture(sunTexture, passUVs.xy).rgb;
	vec3 video_color = texture(videoTexture, passUVs.xy).rgb;
	vec3 weather_color = clamp(cloud_color + sun_color, 0.0, 1.0);

	// set fragment color
	out_Color =  vec4(mix(weather_color, video_color, blendValue), 1.0);
}
