// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#version 450 core

// input  
in float pass_Tip;

// output
out vec4 out_Color;

// All fragment shader ubo's
uniform FRAGUBO
{
	vec3 color;				//< Normal color
	float opacity;			//< Opacity
	float nlength;			//< Normal length
} fubo;

void main() 
{
	float v = 1.0-clamp(pass_Tip, 0.0, 1.0);
	float m = fubo.nlength;
	v =  1.0 - (v / (m) * 1.0);
	out_Color = vec4(fubo.color.rgb, fubo.opacity * v);
}