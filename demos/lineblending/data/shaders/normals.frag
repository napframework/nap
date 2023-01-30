// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#version 450 core

// input  
in float pass_Tip;
in vec4 pass_Color;

// output
out vec4 out_Color;

// Uniform used for setting color
uniform UBO
{
	vec4 mColor;
} ubo;

void main() 
{
	vec3 line_color = ubo.mColor.rgb;
	out_Color = vec4(line_color * pass_Color.rgb, pass_Tip * ubo.mColor.a);
}
