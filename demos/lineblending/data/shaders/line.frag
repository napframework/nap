// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#version 330

// input  
in vec3 pass_Uvs;
in vec4 pass_Color;

// output
out vec4 out_Color;

void main() 
{
	vec3 color = pass_Color.rgb * pass_Color.a;
	out_Color = vec4(color, 1.0);
}
