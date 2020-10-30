// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#version 450 core

// vertex shader input  
in vec3 passUVs;						//< frag Uv's
in vec3 passPosition;					//< frag world space position 

// uniform inputs
uniform sampler2D	inputTexture;		//< input texture

uniform UBO
{
	float			intensity;			//< greyscale intensity value
} ubo;

// output
out vec4 out_Color;

void main() 
{
	// Get texture rgb value
	vec3 tex_color = texture(inputTexture, passUVs.xy).rgb;
	float greyscale = ((tex_color.r + tex_color.g + tex_color.b) / 3.0 * ubo.intensity); 
	out_Color =  vec4(greyscale, greyscale, greyscale, 1.0);
}