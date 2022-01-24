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
	uniform vec3 colorOne;
	uniform vec3 colorTwo;
} ubo;

void main(void)
{
	// Get color from texture
	vec3 color = mix(ubo.colorOne, ubo.colorTwo, pass_Uvs.y);

	// Set output color
	out_Color = vec4(color * ubo.intensity, 1.0);
}