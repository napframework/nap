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
	float displacement;
} ubo;

in vec3	in_Position;
in vec3 in_Normals;

void main(void)
{
	vec3 offset_pos = in_Position + (normalize(in_Normals) * ubo.displacement);
	gl_Position = mvp.projectionMatrix * mvp.viewMatrix * mvp.modelMatrix * vec4(offset_pos, 1.0);
	gl_PointSize = 1.0;
}
