// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#version 450

uniform nap
{
	mat4 projectionMatrix;
	mat4 viewMatrix;
	mat4 modelMatrix;
} mvp;

uniform UBO
{
	vec2 textureSize;	// The size of 'colorTexture', used to pre-calculate sampling coordinates in vertex shader
	vec2 direction;		// The sampling direction
} ubo;

in vec3 in_Position;
in vec3 in_UV0;

out vec2 pass_UVs[5];

// Apply horizontally and vertically for a 9x9 kernel
void main()
{
	vec2 off1 = (vec2(1.3846153846) * ubo.direction) / ubo.textureSize;
	vec2 off2 = (vec2(3.2307692308) * ubo.direction) / ubo.textureSize;

	pass_UVs[0] = in_UV0.xy;
	pass_UVs[1] = in_UV0.xy + off1;
	pass_UVs[2] = in_UV0.xy - off1;
	pass_UVs[3] = in_UV0.xy + off2;
	pass_UVs[4] = in_UV0.xy - off2;

    gl_Position = mvp.projectionMatrix * mvp.viewMatrix * mvp.modelMatrix * vec4(in_Position, 1.0);
}
