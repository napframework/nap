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
	vec3 cameraPosition;
} mvp;


// All uniform vertex shader inputs
uniform VERTUBO
{
	float blendValue;
	float normalBlendValue;
} vubo;


// Input Vertex Attributes
in vec3	in_Position;
in vec3 in_UV0;
in vec3 in_Normal;
in vec3 in_OriginalNormal;
in vec3 in_OriginalPosition;

// Output to fragment shader
out vec3 passUVs;					//< vetex uv's
out vec3 passNormal;				//< vertex normal in world space
out vec3 passPosition;				//< vertex world space position

void main(void)
{
	// Blend original normal with target normal
	vec4 blend_nor = vec4(mix(in_OriginalNormal, in_Normal, vubo.normalBlendValue), 1.0);

	// Blend original position with target position
	vec4 blend_pos = vec4(mix(in_OriginalPosition, in_Position, vubo.blendValue),1.0);

	// Multiply with modelmatrix to position in world space and pass to frag
	passPosition = vec3(mvp.modelMatrix * blend_pos);

	// Rotate normal based on model matrix and set
	passNormal = normalize((mvp.normalMatrix * blend_nor).xyz);

	// Forward uvs to fragment shader
	passUVs = in_UV0;

	// Calculate frag position
    gl_Position = mvp.projectionMatrix * mvp.viewMatrix * mvp.modelMatrix * blend_pos;
}