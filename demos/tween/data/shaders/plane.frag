// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#version 450 core

// vertex shader input  
in vec3 passUVs;						//< frag Uv's
in vec3 passPosition;					//< frag world space position 

// output
out vec4 out_Color;

// constants
const vec3  colorOne  = vec3(0.545, 0.549, 0.627);
const vec3  colorTwo  = vec3(0.176,0.180,0.258);
const float maxOffset = 0.015;

// Uniform inputs
uniform UBO
{
	uniform float 	animationValue;
	uniform vec2	animationPos;
} ubo;

// Maps a value from min, max to outmin and outmax
float fit(float value, float min, float max, float outMin, float outMax)
{
  float v = clamp(value, min, max);
  float m = max - min;
  if(m==0.0)
    m = 0.00000001;
  return (v - min) / (m) * (outMax - outMin) + outMin;
}

void main() 
{
	//
	vec2 center = ubo.animationPos;
	float dist = length(passUVs.xy - center);

	float blend = clamp( 1.0 - ( dist / ( maxOffset * ubo.animationValue ) ), 0, 1);
	if( blend > 0 )
		blend = 1;

	// Find color mix value based on curve (animator) value
	vec3 fcolor = mix(colorTwo, colorOne, blend);

	// Set fragment color output to be texture color
	out_Color =  vec4(fcolor,1.0);
}