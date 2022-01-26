// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#version 450 core

uniform UBO
{
	float contrast;
	float brightness;
} ubo;

in vec3 pass_UV;

out vec4 out_Color;

uniform sampler2D colorTexture;

vec3 brightnessContrast(vec3 color, float brightness, float contrast)
{
    return (color - 0.5) * contrast + 0.5 + brightness;
}

void main(void)
{
	vec4 color = texture(colorTexture, pass_UV.xy);
	out_Color = vec4(brightnessContrast(color.xyz, ubo.brightness, ubo.contrast), color.a);
}
