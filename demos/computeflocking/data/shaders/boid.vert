// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#version 450 core

struct BoidTransform
{
	vec4 translation;
	vec4 rotation;
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
	float boidSize;
};

// STORAGE
layout(std430) buffer TransformBuffer
{
	BoidTransform transforms[1000];
};

// Input Vertex Attributes
in vec3 in_Position;
in vec3 in_Normals;

out vec3 pass_Position;
out vec3 pass_Normals;
//out vec3 pass_Uv;
out uint pass_Id;


// Rotate vector v with quaterion q
vec3 rotate(vec3 v, vec4 q)
{
	float l1 = dot(q.xyz, q.xyz);
	return v * (q.w * q.w - l1) + q.xyz * (dot(v, q.xyz) * 2.0) + cross(q.xyz, v) * (q.w * 2.0);
}


void main(void)
{
	// Get transformation
	BoidTransform bt = transforms[gl_InstanceIndex];
	vec4 position = vec4(boidSize * in_Position, 1.0);
	vec4 world_position = vec4(rotate((mvp.modelMatrix * position).xyz, bt.rotation) + bt.translation.xyz, 1.0);

	// Calculate position
    gl_Position = mvp.projectionMatrix * mvp.viewMatrix * world_position;

	// Calculate vertex world space position and set
	pass_Position = world_position.xyz;

	// Calculate normal in world coordinates and pass along
	vec4 normal = vec4(in_Normals, 0.0);
	pass_Normals = normalize(rotate((mvp.modelMatrix * normal).xyz, bt.rotation));

	// Pass uv's a
	//pass_Uv = in_UV0;

	// Pass element id
	pass_Id = gl_InstanceIndex;
}
