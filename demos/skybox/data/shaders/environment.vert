/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#version 450 core

// Uniforms
uniform nap
{
	mat4 projectionMatrix;
	mat4 viewMatrix;
	mat4 modelMatrix;
	mat4 normalMatrix;
	vec3 cameraPosition;
} mvp;

uniform UBO
{
	vec2	fresnel;						//< Fresnel [scale, power]
} ubo;

// Vertex Input
in vec3		in_Position;					//< Vertex position in object space
in vec3 	in_Normals;						//< Vertex normal in object space

// Vertex Output
out vec3 	passPosition;					//< Vertex position in world space
out vec3 	passNormal;						//< Vertex normal in world space
out float 	passFresnel;					//< Fresnel

void main()
{
	// Calculate frag position
	vec4 world_position = mvp.modelMatrix * vec4(in_Position, 1.0);
	gl_Position = mvp.projectionMatrix * mvp.viewMatrix * world_position;

	passPosition = world_position.xyz;

	// Rotate normal based on model matrix and set
	vec3 world_normal = normalize((mvp.normalMatrix * vec4(in_Normals, 0.0)).xyz);
	passNormal = world_normal;

	// Compute fresnel contribution
	vec3 eye_to_surface = normalize(world_position.xyz - mvp.cameraPosition);
	passFresnel = pow(clamp(1.0 + dot(eye_to_surface, world_normal), 0.0, 1.0), ubo.fresnel.y) * ubo.fresnel.x;
}
