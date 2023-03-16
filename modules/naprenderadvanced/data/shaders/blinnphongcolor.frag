#version 450 core

// Extensions
#extension GL_GOOGLE_include_directive : enable

// Includes
#include "shadow.glslinc"
#include "blinnphong.glslinc"
#include "utils.glslinc"

// Specialization Constants
layout (constant_id = 0) const uint SHADOW_SAMPLE_COUNT = 8;

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
	Light lights[8];
	uint count;
} lit;

uniform UBO
{
	vec3	ambient;				//< Ambient
	vec3	diffuse;				//< Diffuse
	vec3	specular;				//< Specular
	vec2	fresnel;				//< Fresnel [scale, power]
	float	shininess;				//< Shininess
	float	alpha;					//< Alpha
} ubo;

// Fragment Input
in vec3 	passPosition;			//< Fragment position in world space
in vec3 	passNormal;				//< Fragment normal in world space
in vec3 	passUV0;				//< Texture UVs
in float 	passFresnel;			//< Fresnel term

in vec4 	passShadowCoords[8];	//< Shadow Coordinates

// Fragment Output
out vec4 out_Color;

// Shadow Texture Sampler
uniform sampler2DShadow shadowMaps[8];
uniform samplerCube cubeShadowMaps[8];

// Constants
const float SHADOW_STRENGTH = 0.75;


void main()
{
	BlinnPhongMaterial mtl = { ubo.ambient, ubo.diffuse, ubo.specular, ubo.shininess };
	
	// Lights
	vec3 color = computeLights(lit.lights, lit.count, mtl, mvp.cameraPosition, normalize(passNormal), passPosition);
	color = mix(color, vec3(1.0), passFresnel * pow(luminance(color), 0.25));

	// Shadows
	// uint flags[8];
	// for (uint i = 0; i < lit.count; i++)
	// 	flags[i] = lit.lights[i].flags;

	//float shadow = computeShadows(shadowMaps, passShadowCoords, flags, lit.count, min(SHADOW_SAMPLE_COUNT, POISSON_DISK.length())) * SHADOW_STRENGTH;
	//color *= (1.0 - shadow);


	float shadow_result = 0.0;
	for (uint i = 0; i < lit.count; i++)
	{
		uint flags = lit.lights[i].flags;
		if (!hasShadow(flags))
			continue;

		uint map_index = getShadowMapIndex(flags);

		switch (getShadowMapId(flags))
		{
			case SHADOWMAP_QUAD:
			{
				// Perspective divide and map coordinates to [0.0, 1.0] range
				vec3 coord = ((passShadowCoords[i].xyz / passShadowCoords[i].w) + 1.0) * 0.5;
				float bias = 1.0/textureSize(shadowMaps[map_index], 0).x;
				float comp = coord.z - bias;

				// Multi sample
				float shadow = 0.0;
				for (int s=0; s<SHADOW_SAMPLE_COUNT; s++) 
				{
					shadow += 1.0 - texture(shadowMaps[map_index], vec3(coord.xy + POISSON_DISK[s]/SHADOW_POISSON_SPREAD, comp));
				}
				shadow_result = max(shadow / float(SHADOW_SAMPLE_COUNT), shadow_result);
				break;
			}
			case SHADOWMAP_CUBE:
			{
				const float near = 0.1;
				const float far = 50.0;

				// The direction of the light in view space is the sampling coordinate for the cube map
				vec3 coord = normalize(passPosition - lit.lights[i].origin);

				// Measure the depth value of the fragment in the reference frame of the light
				// Ensure the approppriate axis-aligned cube face is used, we derive this from the sampling coordinate
				float frag_depth = sdfPlane(lit.lights[i].origin, cubeFace(coord), passPosition);

				float shadow = 0.0;
				for (int s=0; s<SHADOW_SAMPLE_COUNT; s++) 
				{
					// Add some poisson-based rotational jitter to the sampling vector
					vec2 jitter = POISSON_DISK[s]/SHADOW_POISSON_SPREAD;
					vec3 sample_coord = normalize(rotationMatrix(vec3(1.0, 0.0, 0.0), jitter.x) * rotationMatrix(vec3(0.0, 1.0, 0.0), jitter.y) * vec4(coord, 0.0)).xyz;
		 			float depth01 = texture(cubeShadowMaps[map_index], sample_coord).x;
					float depth = linearDepth(depth01, near, far) * 2.0;

					shadow += (frag_depth <= depth) ? 0.0 : 1.0;
				}
				shadow_result = max(shadow / float(SHADOW_SAMPLE_COUNT), shadow_result);
				break;
			}
		}
	}

	shadow_result = clamp(shadow_result * SHADOW_STRENGTH, 0.0, 1.0);
	color *= (1.0 - shadow_result);

	out_Color = vec4(color, ubo.alpha);
}
