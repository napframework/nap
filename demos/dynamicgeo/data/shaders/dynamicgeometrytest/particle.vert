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

in vec3	in_Position;
in vec4	in_Color0;
in vec3	in_UV0;
in float in_PID;

out vec4 pass_Color;
out vec3 pass_Uvs;
out float pass_PID;

void main(void)
{
	// Calculate position
    gl_Position = mvp.projectionMatrix * mvp.viewMatrix * mvp.modelMatrix * vec4(in_Position, 1.0);

	// Pass color and uv's 
	pass_Color = in_Color0;
	pass_Uvs = in_UV0;
	pass_PID = in_PID;
}