// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#version 450 core

struct Boid
{
	vec4 position;
	vec4 velocity;
	vec4 orientation;

	uint padding_0;
	uint padding_1;
	uint padding_2;

	float mateRate;
};

// UNIFORM
uniform nap
{
	mat4 projectionMatrix;
	mat4 viewMatrix;
	mat4 modelMatrix;
} mvp;

uniform Vert_UBO
{
	vec3 cameraLocation;
	float boidSize;
	float fresnelScale;
	float fresnelPower;
};

// STORAGE
layout(std430) buffer SSBO
{
	Boid boids[10000];
};

// Input Vertex Attributes
in vec3 in_Position;
in vec3 in_Normals;

out vec3 pass_Position;
out vec3 pass_Normal;

out float pass_Fresnel;
out float pass_Mates;
flat out uint pass_Id;


// Rotate vector v with quaterion q
vec3 rotate(vec3 v, vec4 q)
{
	float l1 = dot(q.xyz, q.xyz);
	return v * (q.w * q.w - l1) + q.xyz * (dot(v, q.xyz) * 2.0) + cross(q.xyz, v) * (q.w * 2.0);
}


void main(void)
{
	// Get transformation
	Boid b = boids[gl_InstanceIndex];
	vec4 position = vec4(boidSize * in_Position, 1.0);
	vec4 world_position = vec4(rotate((mvp.modelMatrix * position).xyz, b.orientation) + b.position.xyz, 1.0);

	// Calculate position
	gl_Position = mvp.projectionMatrix * mvp.viewMatrix * world_position;

	// Calculate vertex world space position and set
	pass_Position = world_position.xyz;

	// Calculate normal in world coordinates and pass along
	vec4 normal = vec4(in_Normals, 0.0);
	vec3 world_normal = normalize(rotate((mvp.modelMatrix * normal).xyz, b.orientation));
	pass_Normal = world_normal;

	vec3 eye_to_surface = normalize(world_position.xyz - cameraLocation);
	float fresnel = 0.04 + 0.96 * pow(clamp(1.0 + dot(eye_to_surface, world_normal), 0.0, 1.0), fresnelPower);
	pass_Fresnel = fresnelScale * fresnel;

	// Pass percentage of neighbors (normalized)
	pass_Mates = b.mateRate;

	// Pass element id
	pass_Id = gl_InstanceIndex;
}
