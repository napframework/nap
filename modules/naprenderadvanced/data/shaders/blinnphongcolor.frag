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
uniform samplerCubeShadow cubeShadowMaps[8];

// Constants
const float SHADOW_STRENGTH = 1.0;
const float FRESNEL_STRENGTH = 2.0;
const float MAX_SHADOW_BIAS = 0.005;

void main()
{
	BlinnPhongMaterial mtl = { ubo.ambient, ubo.diffuse, ubo.specular, ubo.shininess };

	vec3 color_result = { 0.0, 0.0, 0.0 };
	for (uint i = 0; i < lit.count; i++)
	{
		vec3 color = computeLight(lit.lights[i], mtl, mvp.cameraPosition, normalize(passNormal), passPosition);
		color = mix(color, vec3(1.0), passFresnel * luminance(color) * FRESNEL_STRENGTH);

		uint flags = lit.lights[i].flags;
		if (!hasShadow(flags))
			continue;

		float shadow = 0.0;
		uint map_index = getShadowMapIndex(flags);
		switch (getShadowMapId(flags))
		{
			case SHADOWMAP_QUAD:
			{
				// Perspective divide and map coordinates to [0.0, 1.0] range
				vec3 coord = ((passShadowCoords[i].xyz / passShadowCoords[i].w));

				// Clip shadow lookups outside of ndc
				if (abs(coord.x) <= 1.0 && abs(coord.y) <= 1.0 && abs(coord.z) <= 1.0)
				{
					// Remap coordinate from ndc [-1, 1] to normalized [0, 1]
					coord.xy = (coord.xy + 1.0) * 0.5;

					// Multi sample
					float sum = 0.0;
					for (int s=0; s<SHADOW_SAMPLE_COUNT; s++) 
					{
						sum += 1.0 - texture(shadowMaps[map_index], vec3(coord.xy + POISSON_DISK[s]/SHADOW_POISSON_SPREAD, coord.z));
					}
					shadow += sum / float(SHADOW_SAMPLE_COUNT);
				}
				break;
			}
			case SHADOWMAP_CUBE:
			{
				// The direction of the light in view space is the sampling coordinate for the cube map
				vec3 coord = normalize(passPosition - lit.lights[i].origin);
				vec2 nf = min(lit.lights[i].nearFar + MAX_SHADOW_BIAS, lit.lights[i].nearFar.y);

				// Measure the depth value of the fragment in the reference frame of the light
				// Ensure the approppriate axis-aligned cube face is used, we derive this from the sampling coordinate
				float frag_depth = sdfPlane(lit.lights[i].origin, cubeFace(coord), passPosition);
				float bias = MAX_SHADOW_BIAS * (1.0 - getSurfaceIncidence(lit.lights[i], passNormal, passPosition));

				float sum = 0.0;
				for (int s=0; s<SHADOW_SAMPLE_COUNT; s++) 
				{
					// Add some poisson-based rotational jitter to the sampling vector
					vec2 jitter = POISSON_DISK[s]/SHADOW_POISSON_SPREAD;
					vec3 sample_coord = normalize(rotationMatrix(vec3(1.0, 0.0, 0.0), jitter.x) * rotationMatrix(vec3(0.0, 1.0, 0.0), jitter.y) * vec4(coord, 0.0)).xyz;
		 			sum += 1.0 - texture(cubeShadowMaps[map_index], vec4(sample_coord, nonLinearDepth(frag_depth - bias, nf.x, nf.y)));
				}
				shadow += sum / float(SHADOW_SAMPLE_COUNT);
				break;
			}
		}
		color_result += color * (1.0-shadow*SHADOW_STRENGTH);
	}
	out_Color = vec4(color_result, ubo.alpha);
}
