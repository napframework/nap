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
const float maxOffset = 0.5; 

// Uniform inputs
uniform UBO
{
	float animationValue;
	vec3  colorOne;
	vec3  colorTwo;
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
	// Find fragment distance to center based on uv coordinates
	vec2 center = vec2(0.5,0.5);
	float dist = length(passUVs.xy - center);

	// Find color mix value based on curve (animator) value
	float mix_value = mix(0.1,maxOffset-0.075, ubo.animationValue);

	// Now mix the two colors based on that
	float color_mix = fit(dist, mix_value,mix_value+0.005, 0.0,1.0);
	vec3 fcolor = mix(ubo.colorOne, ubo.colorTwo, color_mix);

	// Construct alpha value based on distance to center, creates a nice circle
	float alpha = fit(dist, maxOffset-0.005,maxOffset, 1.0,0.0);

	// Set fragment color output to be texture color
	out_Color =  vec4(fcolor,alpha);
}