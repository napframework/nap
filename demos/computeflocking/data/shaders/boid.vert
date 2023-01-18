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
	mat4 normalMatrix;
	vec3 cameraPosition;
} mvp;

uniform VERTUBO
{
	vec3 cameraLocation;
	float boidSize;
	float fresnelScale;
	float fresnelPower;
};

// STORAGE
layout(std430) buffer readonly VERTSSBO
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
	// Use the instance index to read from the boid storage buffer. We make sure the boid count is equal 
	// to the buffer element count in this shader, the vertex shader, and in the buffer descriptor in JSON.
	Boid b = boids[gl_InstanceIndex];

	// Fetch position from vertex attribute
	vec4 position = vec4(boidSize * in_Position, 1.0);

	// Transform the fetched position to world space using the position and orientation of the boid
	vec3 flock_position = rotate(position.xyz, b.orientation) + b.position.xyz;
	vec4 world_position = mvp.modelMatrix * vec4(flock_position, 1.0);

	// Calculate position
	gl_Position = mvp.projectionMatrix * mvp.viewMatrix * world_position;

	// Calculate vertex world space position and set
	pass_Position = world_position.xyz;

	// Calculate normal in world coordinates and pass along
	vec3 flock_normal = normalize(rotate(in_Normals, b.orientation));
	vec3 world_normal = normalize((mvp.modelMatrix * vec4(flock_normal, 0.0)).xyz);
	pass_Normal = world_normal;

	vec3 eye_to_surface = normalize(world_position.xyz - mvp.cameraPosition);
	float fresnel = 0.04 + 0.96 * pow(clamp(1.0 + dot(eye_to_surface, world_normal), 0.0, 1.0), fresnelPower);
	pass_Fresnel = fresnelScale * fresnel;

	// Pass percentage of neighbors (normalized)
	pass_Mates = b.mateRate;

	// Pass element id
	pass_Id = gl_InstanceIndex;
}
