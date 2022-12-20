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
	vec3 cameraWorldPosition;
} mvp;

// Input Vertex Attributes
in vec3	in_Position;
in vec3 in_UV0;
in vec3 in_Normal;

// Output to fragment shader
out vec3 passUVs;					//< vetex uv's
out vec3 passNormal;				//< vertex normal in world space
out vec3 passWorldPosition;			//< vertex world space position

void main(void)
{
	// Calculate vertex world position
	vec4 world_position = (mvp.modelMatrix * vec4(in_Position, 1.0));

	// Calculate frag position
    gl_Position = mvp.projectionMatrix * mvp.viewMatrix * world_position;

	// Set vertex world position
	passWorldPosition = world_position.xyz;

	// Rotate normal based on normal matrix and set
	passNormal = normalize(mvp.normalMatrix * vec4(in_Normal, 0.0)).xyz;

	// Forward uvs to fragment shader
	passUVs = in_UV0;
}