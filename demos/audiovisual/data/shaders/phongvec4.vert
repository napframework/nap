// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#version 450 core

// Extensions
#extension GL_GOOGLE_include_directive : enable

// Total maximum supported number of lights
#include "maxlights.glslinc"

// Includes
#include "light.glslinc"
#include "utils.glslinc"
#include "shadow.glslinc"

uniform nap
{
	mat4 projectionMatrix;
	mat4 viewMatrix;
	mat4 modelMatrix;
	mat4 normalMatrix;
	vec3 cameraPosition;
} mvp;

uniform shadow
{
	mat4 lightViewProjectionMatrix[MAX_LIGHTS];
	vec2 nearFar[MAX_LIGHTS];
	float strength[MAX_LIGHTS];
	uint flags;
	uint count;
} sdw;

uniform UBO
{
	vec4	ambient;						//< Ambient
	vec3	diffuse;						//< Diffuse
	vec3	specular;						//< Specular
	vec3	highlight;						//< Shader specific
	vec3	fresnelColor;					//< Shader specific
	vec2	fresnel;						//< Fresnel [scale, power]
	float	shininess;						//< Shininess
	float	alpha;							//< Alpha
	float	reflection;						//< Reflection
	float	highlightLength;				//< Highlight Length
	uint	environment;					//< Whether to sample an environment map
} ubo;

in vec4	in_Position;
in vec4	in_Normals;
in vec4 in_UV0;

out vec3 passPosition;
out vec3 passNormal;
out vec2 passUV0;
out float passFresnel;
out float passFlux;
out vec4 passShadowCoords[MAX_LIGHTS];	//< Shadow Coordinates


void main(void)
{
	// Calculate frag position
	vec4 world_position = mvp.modelMatrix * vec4(in_Position.xyz, 1.0);
	gl_Position = mvp.projectionMatrix * mvp.viewMatrix * world_position;

	passPosition = world_position.xyz;

	// Rotate normal based on model matrix and set
	vec3 world_normal = normalize((mvp.normalMatrix * in_Normals).xyz);
	passNormal = world_normal;

	// Compute fresnel contribution
	vec3 eye_to_surface = normalize(world_position.xyz - mvp.cameraPosition);
	passFresnel = pow(clamp(1.0 + dot(eye_to_surface, world_normal), 0.0, 1.0), ubo.fresnel.y) * ubo.fresnel.x;

	passUV0 = in_UV0.xy;
	passFlux = in_Position.w;

	// Shadow
	for (uint i = 0; i < min(sdw.count, MAX_LIGHTS); i++)
	{
		// Check if shadow is enabled on this light, else skip
		if (((sdw.flags >> i) & 0x1) != 1)
			continue;

		// Compute current shadow coordinate: the world position in lightviewspace
		vec4 coord = sdw.lightViewProjectionMatrix[i] * world_position;
		
		// Flip y (Vulkan coordinates are [-1, 1], refer to NAP RenderProjectionMatrix)
		coord.y = -coord.y;
		
		// Pass coordinates
		passShadowCoords[i] = coord;
	}
}
