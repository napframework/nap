// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#version 450 core

// matrices ubo
uniform nap
{
	uniform mat4 projectionMatrix;
	uniform mat4 viewMatrix;
	uniform mat4 modelMatrix;
} mvp;

in vec3	in_Position;
in vec4 in_Color0;
in float in_Tip;

out float pass_Tip;
out vec4 pass_Color;

void main(void)
{
	// Calculate position
    gl_Position = mvp.projectionMatrix * mvp.viewMatrix * mvp.modelMatrix * vec4(in_Position, 1.0);

	// Pass tip
	pass_Tip = in_Tip;

	// Pass color
	pass_Color = in_Color0;
}