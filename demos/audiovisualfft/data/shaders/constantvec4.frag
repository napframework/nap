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

uniform UBO
{
	vec3 color;
	float alpha;
} ubo;

in vec2 passUV0;

out vec4 out_Color;

void main() 
{
	out_Color = vec4(ubo.color, ubo.alpha * passUV0.y);
}
 