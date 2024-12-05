// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#version 450 core

// Ensure we register a constant to set
layout(constant_id = 0) const uint CHROMATIC_ABBERATION = 1;

uniform FRAGUBO
{
	float blend;
	float abberation;
} ubo;

in vec3 pass_UV;

out vec4 out_Color;

uniform sampler2D colorTexture;

vec4 chromatic_abberation_sample(sampler2D tex, vec2 uv)
{
	const vec2 ctr = { 0.5, 0.5 };
	const ivec2 res = textureSize(tex, 0);
	vec2 diff = uv - ctr;
	float intensity = clamp(length(diff), 0.0, 1.0);
	vec2 d = normalize(diff) * intensity * ubo.abberation;

	vec4 col = vec4(0.0);
	col.r = texture(tex, uv + d).r;
	col.ga = texture(tex, uv).ga;
	col.b = texture(tex, uv - d).b;

	return col;
}

void main(void)
{	
	// Get texel color values
	vec4 col = (CHROMATIC_ABBERATION > 0) ? 
		chromatic_abberation_sample(colorTexture, pass_UV.xy) :
		texture(colorTexture, pass_UV.xy);

	out_Color = vec4(col.rgb, 1.0);
}
