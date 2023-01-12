// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#version 450 core

uniform nap
{
	mat4 projectionMatrix;
	mat4 viewMatrix;
	mat4 modelMatrix;
	mat4 normalMatrix;
	vec3 cameraPosition;
} mvp;

// Input attributes
in vec3	in_Position;		// Vertex position
in vec3 in_Normals;			// Vertex normal
in vec3	in_UV0;				// Vertex uv

// Output attributes to fragment shader
out vec3 pass_Uvs0;
out mat4 pass_ModelMatrix;
out vec3 pass_Vert;
out vec3 pass_Normals;

void main(void)
{
	// Calculate position
    gl_Position = mvp.projectionMatrix * mvp.viewMatrix * mvp.modelMatrix * vec4(in_Position, 1.0);

	// Pass color and uv's 
	pass_Uvs0 = in_UV0;
	pass_Vert = in_Position;
	pass_ModelMatrix = mvp.modelMatrix;
	pass_Normals = in_Normals;
}