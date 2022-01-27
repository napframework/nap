// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#version 450 core

uniform UBO
{
	float blend;
	//float exposure;
} ubo;

in vec3 pass_UV;

out vec4 out_Color;

uniform sampler2D colorTextures[2];

void main(void)
{
	vec4 col0 = texture(colorTextures[0], pass_UV.xy);
	vec4 col1 = texture(colorTextures[1], pass_UV.xy);

	// Tone mapping
	// vec4 hdr = col0 + col1;
	// vec3 tone_mapped = vec3(1.0) - exp(-hdr.rgb * ubo.exposure);
	// out_Color = vec4(tone_mapped, 1.0);

	// Blend
	vec3 color = col0.rgb * (1.0 - ubo.blend) + col1.rgb * ubo.blend;
	out_Color = vec4(color, 1.0);
}
