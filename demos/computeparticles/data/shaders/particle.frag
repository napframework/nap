// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#version 450 core

in vec4 pass_Uvs;
flat in uint pass_Id;

out vec4 out_Color;

uniform sampler2D texture_input[2];

const float EPSILON = 0.000001;

void main(void)
{	
	// Check which image to get
	// This is a hack to ensure this demo works on linux with the mesa drivers
	// Otherwise: ERROR: sampler arrays indexed with non-constant expressions are forbidden in GLSL 1.30 and later
	vec4 tex_color = vec4(0,0,0,0);
	if(pass_Id == 0)
	{
		tex_color = texture(texture_input[0], pass_Uvs.xy);
	}
	else
	{
		tex_color = texture(texture_input[1], pass_Uvs.xy);
	}

	if (tex_color.a <= 1.0-EPSILON)
		discard;

	// Set output color
	out_Color = tex_color;
}
