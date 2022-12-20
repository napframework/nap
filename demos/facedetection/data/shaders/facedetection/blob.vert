// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#version 450 core

// NAP Uniforms
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
in vec3 in_Normals;

// Output to fragment shader
out vec3 passNormals;				//< Vertex normal
out vec3 passVert;					//< Vertex position in object space 

void main(void)
{
	// Pass along normals for light calculations
	passNormals = in_Normals;

	// Pass along vertex position in object space
	passVert = in_Position;

	// Calculate frag position
    gl_Position = mvp.projectionMatrix * mvp.viewMatrix * mvp.modelMatrix * vec4(in_Position, 1.0);
}