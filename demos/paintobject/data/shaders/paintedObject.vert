/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#version 450 core

uniform nap
{
	uniform mat4 projectionMatrix;
	uniform mat4 viewMatrix;
	uniform mat4 modelMatrix;
} mvp;

// Input Vertex Attributes
in vec3	in_Position;
in vec3 in_UV0;
in vec3 in_Normal;

// Output to fragment shader
out vec3 passUVs;					//< vetex uv's
out vec3 passNormal;				//< vertex normal in world space
out vec3 passPosition;				//< vertex world space position
out mat4 passModelMatrix;			//< Matrix to transform vertex from object to world space
out vec3 cameraLocation;			//< camera location
out vec3 passVert;					//< Vertex position in object space 

void main(void)
{
	// Pass along model matrix for light calculations
	passModelMatrix = mvp.modelMatrix;

	// Calculate frag position
    gl_Position = mvp.projectionMatrix * mvp.viewMatrix * mvp.modelMatrix * vec4(in_Position, 1.0);

	//rotate normal based on model matrix and set
    mat3 normal_matrix = transpose(inverse(mat3(mvp.modelMatrix)));
	passNormal = normalize(normal_matrix * in_Normal);

	// calculate vertex world space position and set
	passPosition = vec3(mvp.modelMatrix * vec4(in_Position, 1));

	// Forward uvs to fragment shader
	passUVs = in_UV0;

	// Extract camera location
	cameraLocation = vec3(inverse(mvp.viewMatrix)[3]);

	// Pass along vertex position in object space
	passVert = in_Position;
}