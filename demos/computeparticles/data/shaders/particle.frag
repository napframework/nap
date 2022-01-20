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
	// This is a hack to ensure this demo works on linux with the mesa opengl drivers
	// Otherwise: OpenGL ERROR: sampler arrays indexed with non-constant expressions are forbidden in GLSL 1.30 and later
	uint tex_id = uint(pass_Id+0.1) % 2;

	vec4 tex_color = texture(texture_input[tex_id], pass_Uvs.xy);
	if (tex_color.a == 0.0)
		discard;

	// Set output color
	out_Color = tex_color;
}
