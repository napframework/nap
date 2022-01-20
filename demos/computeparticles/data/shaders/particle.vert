// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#version 450 core

uniform nap
{
	mat4 projectionMatrix;
	mat4 viewMatrix;
	mat4 modelMatrix;
} mvp;

in vec4	in_Position;
in vec4	in_UV0;
in uint in_Id;

out vec4 pass_Uvs;
out uint pass_Id;

void main(void)
{
	// Calculate position
    gl_Position = mvp.projectionMatrix * mvp.viewMatrix * mvp.modelMatrix * vec4(in_Position.xyz, 1.0);

	// Pass color and uv's 
	pass_Uvs = in_UV0;

	// Pass element id
	pass_Id = in_Id;
}