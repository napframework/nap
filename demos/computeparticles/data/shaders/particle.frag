// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#version 450 core

in vec4 pass_Uvs;
flat in uint pass_Id;

out vec4 out_Color;

uniform sampler2D texture_input[2];

void main(void)
{	
	vec4 tex_color = texture(texture_input[pass_Id], pass_Uvs.xy);
	if (tex_color.a == 0.0)
		discard;

	// Set output color
	out_Color = tex_color;
}
