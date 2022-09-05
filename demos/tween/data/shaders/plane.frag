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
const float maxOffset = 0.015;

// Uniform inputs
uniform UBO
{
	uniform float  	animationValue;
	uniform vec2 		animationPos;
	uniform vec3   	colorOne;
	uniform vec3   	colorTwo;
} ubo;

// Maps a value from min, max to outmin and outmax
float fit(float value, float min, float max, float outMin, float outMax)
{
  float v = clamp(value, min, max);
  float m = max - min;
  m = m == 0.0 ? 0.00000001 : m;
  return (v - min) / (m) * (outMax - outMin) + outMin;
}

void main() 
{
	// Get distance to current animation position
	vec2 center = ubo.animationPos;
	float dist = length(passUVs.xy - center);

	// Compute blend value
	float blend = fit(dist, maxOffset-0.0002, maxOffset, 1.0, 0.0);

	// Find color mix value based on curve (animator) value
	vec3 fcolor = mix(ubo.colorTwo, ubo.colorOne, blend);

	// Set fragment color output to be texture color
	out_Color =  vec4(fcolor,1.0);
}