// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#version 450 core

uniform UBO
{
	float mixValue;
} ubo;

in vec3 pass_UV;

out vec4 out_Color;

uniform sampler2D colorTextures[2];

void main(void)
{
	vec4 col0 = texture(colorTextures[0], pass_UV.xy);
	vec4 col1 = texture(colorTextures[1], pass_UV.xy);

	// Blend
	out_Color = col0 * (1.0 - ubo.mixValue) + col1 * ubo.mixValue;
}
