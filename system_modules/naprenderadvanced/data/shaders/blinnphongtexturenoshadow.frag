#version 450 core

// Extensions
#extension GL_GOOGLE_include_directive : enable

// Total maximum supported number of lights
#include "maxlights.glslinc"

// Includes
#include "blinnphongutils.glslinc"
#include "utils.glslinc"

// Specialization constants
layout (constant_id = 0) const uint QUAD_SAMPLE_COUNT = 8;
layout (constant_id = 1) const uint CUBE_SAMPLE_COUNT = 4;
layout (constant_id = 2) const uint ENABLE_ENVIRONMENT_MAPPING = 1;

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

uniform UBO
{ 
	vec4	ambient;						//< Ambient
	vec3	diffuse;						//< Diffuse
	vec3	specular;						//< Specular
	vec2	fresnel;						//< Fresnel [scale, power]
	float	shininess;						//< Shininess
	float	alpha;							//< Alpha
	uint	environment;					//< Whether to sample an environment map
} ubo;

// Fragment Input
in vec3 	passPosition;					//< Fragment position in world space
in vec3 	passNormal;						//< Fragment normal in world space
in vec3 	passUV0;						//< Texture UVs
in float 	passFresnel;					//< Fresnel term

// Fragment Output
out vec4 out_Color;

uniform sampler2D colorTexture;
uniform samplerCube environmentMap;

void main()
{
	// Material color
	vec4 texture_color = texture(colorTexture, passUV0.xy);
	BlinnPhongMaterial mtl = { ubo.ambient, texture_color.rgb * ubo.diffuse, ubo.specular, ubo.shininess };

	// Sample environment map
	if (ENABLE_ENVIRONMENT_MAPPING > 0 && ubo.environment > 0)
	{
		vec3 I = normalize(passPosition - mvp.cameraPosition);
		vec3 R = reflect(I, normalize(passNormal));
		mtl.diffuse *= mix(mtl.diffuse, texture(environmentMap, R).rgb, 1.0);
	}

	// Compute light contribution
	vec3 color_result = { 0.0, 0.0, 0.0 };
	for (uint i = 0; i < min(lit.count, MAX_LIGHTS); i++)
	{
		// Skip light and shadow computation if intensity is zero
		if (lit.lights[i].intensity <= EPSILON || !isLightEnabled(lit.lights[i].flags))
			continue;

		// Lights
		color_result += computeLight(lit.lights[i], mtl, mvp.cameraPosition, normalize(passNormal), passPosition);
	}

	// Add fresnel
	color_result = mix(color_result, vec3(1.0), passFresnel);

	// Final color output
	out_Color = vec4(color_result, ubo.alpha);
}
