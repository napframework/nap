// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#version 450 core

// Uniforms
uniform nap
{
	mat4 projectionMatrix;
	mat4 viewMatrix;
	mat4 modelMatrix;
} mvp;

// Vertex Input
in vec3	in_Position;

void main()
{
	// Negate translational component
	mat4 view_matrix = mvp.viewMatrix;
	view_matrix[3] = vec4(0.0, 0.0, 0.0, 1.0);

	// Model matrix may only have scaling
	gl_Position = mvp.projectionMatrix * view_matrix * mvp.modelMatrix * vec4(in_Position.xyz, 1.0);
	gl_PointSize = 1.0;
}