#version 450 core

// Extensions
#extension GL_GOOGLE_include_directive : enable

// Includes
#include "shadow.glslinc"
#include "blinnphong.glslinc"

// Test
#include "noise.glslinc"
#include "utils.glslinc"

// vertex shader input
in vec3 passPosition;					//< frag position in object space
in vec3 passNormal;						//< frag normal in object space
in vec3 passUV0;						//< UVs
in float passFresnel;					//< fresnel term
in vec4 passShadowCoord[8];				//< shadow coord

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
	DirectionalLightShadow lights[8];
	uint count;
} lit;

// uniform inputs
uniform UBO
{
	vec3 	color;						//< Color
	float	alpha;						//< Alpha
	float	bias;
} ubo;

// Shader Output
out vec4 out_Color;

// Shadow Texture Sampler
uniform sampler2DShadow shadowMaps[8];

// Constants
const float SHADOW_STRENGTH = 0.9;


void main()
{
	vec3 color = computeLight(lit.lights, lit.count, mvp.cameraPosition, ubo.color, normalize(passNormal), passPosition);
	color = mix(color, vec3(0.975), passFresnel);

	float shadow = computeShadow(shadowMaps, passShadowCoord, lit.count) * SHADOW_STRENGTH;
	color *= (1.0 - shadow);

	out_Color = vec4(color, ubo.alpha);
}
