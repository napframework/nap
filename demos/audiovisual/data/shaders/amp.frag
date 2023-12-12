// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#version 450 core

uniform nap
{
	mat4 projectionMatrix;
	mat4 viewMatrix;
	mat4 modelMatrix;
} mvp;

// input vertex attributes
in vec3 passUVs;

// output
out vec4 out_Color;

const float EPSILON = 0.0001;
const float PI = 3.141592;
const float SMOOTH = 0.002;


float fit(float value, float inMin, float inMax, float outMin, float outMax)
{
	float v = clamp(value, inMin, inMax);
	return (v - inMin) / (inMax - inMin) * (outMax - outMin) + outMin;
}


void main() 
{
	// int ix = int(fit(passUVs.x, 0.0, 1.0, 1.0, ubo.amps.length()-1));
	// float y = ubo.amps[ix];
	// float pct = step(passUVs.y / (ix+1), y); 

	vec2 uv = passUVs.xy;
	vec2 to_center = vec2(0.5) - uv;
	float frag_angle = -atan(to_center.y, to_center.x);
	float a_norm = fit(frag_angle, -PI, PI, 0.0, 1.0);
	float x = fit(a_norm, 0.0, 1.0, 1.0, ubo.amps.length()-1);
	int ix = int(x);
	float amp = (ubo.amps[ix] * (1.0+x)) * 0.5;
	float frag_radius = -0.1 + length(to_center) * 1.0;
	float pct = smoothstep(frag_radius-SMOOTH, frag_radius+SMOOTH, amp); 

	vec3 color = vec3(0.1);
	out_Color = vec4(color, 1.0); 
}
