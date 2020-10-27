// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#version 450 core

in vec3 pass_Uvs;
out vec4 out_Color;

// Uniform buffer inputs
uniform UBO
{
	uniform float intensity;
} ubo;

// Samplers
uniform sampler2D	backgroundTexture;

void main(void)
{
	// Get color from texture
	vec3 color = texture(backgroundTexture, pass_Uvs.xy).rgb;

	// Set output color
	out_Color = vec4(color * ubo.intensity, 1.0);
}