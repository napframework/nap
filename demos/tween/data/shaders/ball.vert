// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#version 450 core

// Uniform inputs
uniform nap
{
	mat4 projectionMatrix;
	mat4 viewMatrix;
	mat4 modelMatrix;
	mat4 normalMatrix;
	vec3 cameraWorldPosition;
} mvp;

in vec3	in_Position;
in vec3	in_UV0;
in vec4 in_Color0;
in vec3 in_Normal;

// Output to fragment shader
out vec3 passUVs;					//< vertex uv's
out vec3 passNormal;				//< vertex normal in object space
out vec3 passPosition;				//< vertex position in object space

void main(void)
{
	// Calculate frag position
    gl_Position = mvp.projectionMatrix * mvp.viewMatrix * mvp.modelMatrix * vec4(in_Position, 1.0);

    // Pass normal in object space
	passNormal = in_Normal;

	// Pass position in object space
	passPosition = in_Position;

	// Forward uvs to fragment shader
	passUVs = in_UV0;
}