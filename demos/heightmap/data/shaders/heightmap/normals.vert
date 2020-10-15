// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#version 450 core

// All model view projection inputs
uniform nap
{
	uniform mat4 projectionMatrix;
	uniform mat4 viewMatrix;
	uniform mat4 modelMatrix;
} mvp;

// All vert shader uniform inputs
uniform VERTUBO
{
	uniform float blendValue;
	uniform float normalBlendValue;
} vubo;


in vec3	in_Position;
in float in_Tip;
in vec3 in_OriginalPosition;
in vec3 in_DisplacedPosition;

out float pass_Tip;

void main(void)
{
	// Blend the non-displacement with the displacement normal based on the blend value
	vec3 blend_dis = mix(in_DisplacedPosition, in_Position, vubo.normalBlendValue);

	// Blend original position with target position
	vec3 blend_pos = mix(in_OriginalPosition, blend_dis, vubo.blendValue);

	// Calculate position
    gl_Position = mvp.projectionMatrix * mvp.viewMatrix * mvp.modelMatrix * vec4(blend_pos, 1.0);

	// Pass color
	pass_Tip = in_Tip;
}