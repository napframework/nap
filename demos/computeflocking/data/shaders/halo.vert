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
};

// Input Vertex Attributes
in vec3 in_Position;
in vec3 in_Normals;

// Output
out float pass_Fresnel;

void main(void)
{
	// Calculate position
	vec4 position = vec4(in_Position, 1.0);
    gl_Position = mvp.projectionMatrix * mvp.viewMatrix * mvp.modelMatrix * position;

	// Calculate vertex world space position
	vec4 world_position = mvp.modelMatrix * position;

	// Calculate normal in world coordinates and pass along
	vec4 normal = vec4(in_Normals, 0.0);
	vec3 world_normal = normalize(mvp.modelMatrix * normal).xyz;

	// Calculate fresnel term
	vec3 eye_to_surface = normalize(world_position.xyz - cameraLocation);
	pass_Fresnel = fresnelScale * pow(1.0 + dot(eye_to_surface, world_normal), fresnelPower);
}
