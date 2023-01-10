// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#version 450 core

// Vertex shader model matrix uniforms
uniform nap
{
	mat4 projectionMatrix;
	mat4 viewMatrix;
	mat4 modelMatrix;
	mat4 normalMatrix;
	vec3 cameraPosition;
} mvp;

// Input attributes
in vec3	in_Position;			// Vertex position
in vec3 in_Normals;				// Vertex normal
in vec3	in_UV0;					// Vertex uv
in vec3 in_Tangent;				// Vertex tangent
in vec3 in_Bitangent;			// Vertex bi-tangent

// Output attributes to fragment shader
out vec3 pass_Uvs0;
out vec3 pass_Normals;
out mat4 pass_ModelMatrix;
out vec3 pass_Vert;
out vec3 pass_Tangent;
out vec3 pass_Bitangent;

void main(void)
{
	// Calculate position
    gl_Position = mvp.projectionMatrix * mvp.viewMatrix * mvp.modelMatrix * vec4(in_Position, 1.0);

	// Pass all important attributes to frag shader
	pass_Uvs0 = in_UV0;
	pass_Normals = in_Normals;
	pass_ModelMatrix = mvp.modelMatrix;
	pass_Vert = in_Position;
	pass_Tangent = in_Tangent;
	pass_Bitangent = in_Bitangent;
}