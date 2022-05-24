// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#version 450 core

uniform nap
{
	mat4 projectionMatrix;
	mat4 viewMatrix;
	mat4 modelMatrix;
} mvp;

uniform VERTUBO
{
	vec3 cameraLocation;
	float fresnelScale;
	float fresnelPower;
} vubo;

// Input Vertex Attributes
in vec3 in_Position;
in vec3 in_Normals;
in vec3 in_UV0;

out vec3 pass_Position;
out vec3 pass_Normal;
out vec3 pass_UV0;
out float pass_Fresnel;


void main(void)
{
	// rotate normal based on model matrix and set
	mat3 normal_matrix = transpose(inverse(mat3(mvp.modelMatrix)));
	pass_Normal = normalize(normal_matrix * in_Normals);

	// calculate vertex world space position and set
	pass_Position = vec3(mvp.modelMatrix * vec4(in_Position, 1.0));

	// Calculate frag position
	gl_Position = mvp.projectionMatrix * mvp.viewMatrix * vec4(pass_Position, 1.0);

	pass_UV0 = in_UV0;

	vec3 eye_to_surface = normalize(pass_Position - vubo.cameraLocation);
	float fresnel = 0.04 + 0.96 * pow(clamp(1.0 + dot(eye_to_surface, pass_Normal), 0.0, 1.0), vubo.fresnelPower);
	pass_Fresnel = vubo.fresnelScale * fresnel;
}
