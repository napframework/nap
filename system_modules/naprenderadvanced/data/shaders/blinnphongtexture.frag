/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#version 450 core

// Extensions
#extension GL_GOOGLE_include_directive : enable

// Total maximum supported number of lights
#include "maxlights.glslinc"

// Includes
#include "shadow.glslinc"
#include "blinnphongutils.glslinc"
#include "utils.glslinc"

// Specialization constants
layout (constant_id = 0) const uint QUAD_SAMPLE_COUNT = 8;
layout (constant_id = 1) const uint CUBE_SAMPLE_COUNT = 4;
layout (constant_id = 2) const uint ENABLE_ENVIRONMENT_MAPPING = 1;
layout (constant_id = 3) const uint DIFFUSE_ONLY = 0;

// Uniforms
uniform nap
{
	mat4 projectionMatrix;
	mat4 viewMatrix;
	mat4 modelMatrix;
	mat4 normalMatrix;
	vec3 cameraPosition;
} mvp;

uniform light
{
	Light lights[MAX_LIGHTS];
	uint count;
} lit;

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
	vec3	ambient;						//< Ambient
	vec3	diffuse;						//< Diffuse
	vec3	specular;						//< Specular
	vec2	fresnel;						//< Fresnel [scale, power]
	float	shininess;						//< Shininess
	float	alpha;							//< Alpha
	float	reflection;						//< Reflection	
	uint	environment;					//< Whether to sample an environment map
} ubo;

// Fragment Input
in vec3 	passPosition;					//< Fragment position in world space
in vec3 	passNormal;						//< Fragment normal in world space
in vec3 	passUV0;						//< Texture UVs
in float 	passFresnel;					//< Fresnel term
in vec4 	passShadowCoords[MAX_LIGHTS];	//< Shadow Coordinates

// Fragment Output
out vec4 out_Color;

// Shadow Texture Sampler
uniform sampler2DShadow shadowMaps[MAX_LIGHTS];
uniform samplerCubeShadow cubeShadowMaps[MAX_LIGHTS];
uniform samplerCube environmentMap;

uniform sampler2D colorTexture;

void main()
{
	// Diffuse-only early exit for debugging purposes
	if (DIFFUSE_ONLY > 0)
	{
		out_Color = vec4(ubo.diffuse, ubo.alpha);
		return;
	}
	
	// Material color
	vec4 texture_color = texture(colorTexture, passUV0.xy);
	BlinnPhongMaterial mtl = { ubo.ambient, texture_color.rgb * ubo.diffuse, ubo.specular, ubo.shininess };

	// Sample environment map
	if (ENABLE_ENVIRONMENT_MAPPING > 0 && ubo.environment > 0)
	{
		vec3 I = normalize(passPosition - mvp.cameraPosition);
		vec3 R = reflect(I, normalize(passNormal));
		mtl.diffuse *= mix(mtl.diffuse, texture(environmentMap, R).rgb, ubo.reflection);
	}

	// Compute light contribution
	vec3 color_result = { 0.0, 0.0, 0.0 };
	for (uint i = 0; i < min(lit.count, MAX_LIGHTS); i++)
	{
		// Skip light and shadow computation if intensity is zero
		if (lit.lights[i].intensity <= EPSILON || !isLightEnabled(lit.lights[i].flags))
			continue;

		// Lights
		vec3 color = computeLight(lit.lights[i], mtl, mvp.cameraPosition, normalize(passNormal), passPosition);

		// Shadows
		uint flags = lit.lights[i].flags;
		if (!hasShadow(flags) || !isShadowEnabled(lit.lights[i].flags))
		{
			color_result += color;
			continue;
		}

		float shadow = 0.0;
		switch (getShadowMapType(flags))
		{
			case SHADOWMAP_QUAD:
			{
				// Apply perspective divide if required
				vec3 coord = passShadowCoords[i].xyz / passShadowCoords[i].w;

				// Clip shadow lookups outside of ndc
				if (abs(coord.x) <= 1.0 && abs(coord.y) <= 1.0 && abs(coord.z) <= 1.0)
				{
					// Remap coordinate from ndc [-1, 1] to normalized [0, 1]
					coord.xy = (coord.xy + 1.0) * 0.5;

					// Multi sample
					const uint map_index = getShadowMapIndex(flags);
					float sum = 0.0;
					for (int s=0; s<QUAD_SAMPLE_COUNT; s++) 
					{
						sum += 1.0 - texture(shadowMaps[map_index], vec3(coord.xy + POISSON_DISK[s]/SHADOW_POISSON_SPREAD, coord.z));
					}
					shadow += sum / float(QUAD_SAMPLE_COUNT);
				}
				break;
			}
			case SHADOWMAP_CUBE:
			{
				// The direction of the light in view space is the sampling coordinate for the cube map
				vec3 coord = normalize(passPosition - lit.lights[i].origin);

				// Adding this small constant to resolve sampling artifacts in cube seams
				vec2 nf = min(sdw.nearFar[i] + 0.001, sdw.nearFar[i].y);

				// Measure the depth value of the fragment in the reference frame of the light
				// Ensure the approppriate axis-aligned cube face is used, we derive this from the sampling coordinate
				float frag_depth = sdfPlane(lit.lights[i].origin, cubeFace(coord), passPosition);

				const uint map_index = getShadowMapIndex(flags);
				float sum = 0.0;
				for (int s=0; s<CUBE_SAMPLE_COUNT; s++) 
				{
					// Add some poisson-based rotational jitter to the sampling vector
					vec2 jitter = POISSON_DISK[s]/SHADOW_POISSON_SPREAD;
					vec3 sample_coord = normalize(rotationMatrix(vec3(1.0, 0.0, 0.0), jitter.x) * rotationMatrix(vec3(0.0, 1.0, 0.0), jitter.y) * vec4(coord, 0.0)).xyz;
		 			sum += 1.0 - texture(cubeShadowMaps[map_index], vec4(sample_coord, nonLinearDepth(frag_depth, nf.x, nf.y)));
				}
				shadow += sum / float(CUBE_SAMPLE_COUNT);
				break;
			}
		}
		color_result += color * (1.0 - shadow * sdw.strength[i]);
	}

	// Add fresnel
	color_result = mix(color_result, vec3(1.0), passFresnel) + mtl.ambient;

	// Final color output
	out_Color = vec4(color_result, ubo.alpha);
}
