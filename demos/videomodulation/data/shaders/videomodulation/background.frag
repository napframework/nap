// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#version 450 core

// vertex shader input  
in vec3 passUVs;						//< frag Uv's
in vec3 passPosition;					//< frag world space position 

// uniform inputs
uniform sampler2D	videoTexture;

uniform UBO
{
	vec3		colorOne;
	vec3		colorTwo;
} ubo;

// output
out vec4 out_Color;

void main() 
{
	// Get texture rgba value
	vec3 tex_color = texture(videoTexture, passUVs.xy).rgb;

	float greyscale = (tex_color.r + tex_color.g + tex_color.b) / 3.0; 
	vec3 out_color = mix(ubo.colorOne, ubo.colorTwo, greyscale);

	// Set fragment color output to be texture color
	out_Color =  vec4(out_color, 1.0);
}