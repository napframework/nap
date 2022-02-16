// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#version 450 core

uniform UBO
{
	vec3 color;
	float alpha;
} ubo;

in float pass_Fresnel;
out vec4 out_Color;

void main(void)
{
	out_Color = vec4(ubo.color, clamp(pass_Fresnel * ubo.alpha, 0.0, 1.0)); 
}
