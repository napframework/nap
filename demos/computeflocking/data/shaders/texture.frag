// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#version 450 core

in vec3 pass_Uvs;

out vec4 out_Color;

uniform sampler2D colorTexture;

void main(void)
{
	// Set output color
	out_Color = texture(colorTexture, pass_Uvs.xy);
}
