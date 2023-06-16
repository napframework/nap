// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#version 450 core

// vertex shader input  
in vec3 passUVs;						//< frag Uv's

uniform nap
{
	mat4 projectionMatrix;
	mat4 viewMatrix;
	mat4 modelMatrix;
	mat4 normalMatrix;
	vec3 cameraPosition;
} mvp;

// uniform buffer inputs
uniform UBO
{
	vec3 color;						//< Cube color
} ubo;

// unfiorm sampler inputs 
uniform sampler2D inTexture;		//< Cube texture

// output
out vec4 out_Color;

void main() 
{
	// Use texture alpha to blend between two colors
	vec3 color = texture(inTexture, passUVs.xy).rgb;
	color *= ubo.color;

	// Set fragment color output
	out_Color =  vec4(color,1.0);
}
