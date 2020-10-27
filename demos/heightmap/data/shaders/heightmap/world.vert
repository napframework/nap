// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#version 450 core

// All model view projection inputs
uniform nap
{
	uniform mat4 projectionMatrix;
	uniform mat4 viewMatrix;
	uniform mat4 modelMatrix;
} mvp;

// All uniform vertex shader inputs
uniform VERTUBO
{
	uniform float blendValue;
	uniform float normalBlendValue;
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
	vec3 blend_nor = mix(in_OriginalNormal, in_Normal, vubo.normalBlendValue);

	//rotate normal based on model matrix and set
    mat3 normal_matrix = transpose(inverse(mat3(mvp.modelMatrix)));
	passNormal = normalize(normal_matrix * blend_nor);

	// Blend original position with target position
	vec3 blend_pos = mix(in_OriginalPosition, in_Position, vubo.blendValue);

	// Multiply with modelmatrix to position in world space and pass to frag
	passPosition = vec3(mvp.modelMatrix * vec4(blend_pos, 1));

	// Forward uvs to fragment shader
	passUVs = in_UV0;

	// Calculate frag position
    gl_Position = mvp.projectionMatrix * mvp.viewMatrix * mvp.modelMatrix * vec4(blend_pos, 1.0);
}