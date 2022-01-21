// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#version 450 core

// UNIFORM
uniform nap
{
	mat4 projectionMatrix;
	mat4 viewMatrix;
	mat4 modelMatrix;
} mvp;

uniform Vert_UBO
{
	float boidSize;
};

// STORAGE
layout(std430) buffer MatrixBuffer
{
	mat4 transforms[1000];
};

// Input Vertex Attributes
in vec3 in_Position;
in vec3 in_Normals;

out vec3 pass_Position;
out vec3 pass_Normals;
//out vec3 pass_Uv;
out int pass_Id;

void main(void)
{
	// Get transformation
	mat4 trans = transforms[gl_InstanceIndex];
	vec4 position = vec4(boidSize * in_Position, 1.0);

	// Calculate position
    gl_Position = mvp.projectionMatrix * mvp.viewMatrix * trans * mvp.modelMatrix * position;

	// calculate vertex world space position and set
	pass_Position = vec3(trans * mvp.modelMatrix * position);

	// calculate normal in world coordinates and pass along
    mat3 normal_matrix = transpose(inverse(mat3(trans * mvp.modelMatrix)));
	pass_Normals = normalize(normal_matrix * in_Normals);

	// Pass uv's 
	//pass_Uv = in_UV0;

	// Pass element id
	pass_Id = gl_InstanceIndex;
}