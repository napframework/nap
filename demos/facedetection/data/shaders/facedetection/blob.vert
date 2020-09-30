// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#version 450 core

// NAP specific matrices
uniform nap
{
	uniform mat4 projectionMatrix;
	uniform mat4 viewMatrix;
	uniform mat4 modelMatrix;
} mvp;

// Input Vertex Attributes
in vec3	in_Position;
in vec3 in_Normals;

// Output to fragment shader
out vec3 passNormals;				//< Vertex normal
out mat4 passModelMatrix;			//< Matrix to transform vertex from object to world space
out vec3 passVert;					//< Vertex position in object space 
out vec3 cameraLocation;			//< camera location

void main(void)
{
	// Pass along model matrix for light calculations
	passModelMatrix = mvp.modelMatrix;

	// Pass along normals for light calculations
	passNormals = in_Normals;

	// Pass along vertex position in object space
	passVert = in_Position;

	// Extract camera location
	cameraLocation = vec3(inverse(mvp.viewMatrix)[3]);

	// Calculate frag position
    gl_Position = mvp.projectionMatrix * mvp.viewMatrix * mvp.modelMatrix * vec4(in_Position, 1.0);
}