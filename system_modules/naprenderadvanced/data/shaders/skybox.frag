// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#version 450 core

// Uniforms
uniform UBO
{
	vec3 color;
} ubo;

// Fragment Input
in vec3 passPosition;				//< Fragment position in world space

// Fragment Output
out vec4 out_Color;

// Texture Sampler
uniform samplerCube cubeTexture;

void main()
{
	vec4 cube = texture(cubeTexture, normalize(passPosition));
	out_Color = vec4(cube.rgb * ubo.color, cube.a);
}
