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
	vec3 cameraPosition;
} mvp;

in vec4	in_Position;
in vec4	in_Normals;
in vec4 in_UV0;

out vec3 passPosition;
out vec3 passNormal;
out vec2 passUV0;
out float passFresnel;
out float passFlux;

const vec2 fresnel = { 0.75, 6.0 };

void main(void)
{
	vec4 world_position = mvp.modelMatrix * vec4(in_Position.xyz, 1.0);
	passPosition = world_position.xyz;

	// Rotate normal based on model matrix and set
	vec3 world_normal = normalize((mvp.normalMatrix * in_Normals).xyz);
	passNormal = world_normal;

	passUV0 = in_UV0.xy;
	passFlux = in_Position.w;

	// Compute fresnel contribution
	vec3 eye_to_surface = normalize(world_position.xyz - mvp.cameraPosition);
	passFresnel = pow(clamp(1.0 + dot(eye_to_surface, world_normal), 0.0, 1.0), fresnel.y) * fresnel.x;

	gl_Position = mvp.projectionMatrix * mvp.viewMatrix * world_position;
}
