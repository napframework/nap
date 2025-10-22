// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#version 450 core

// Extensions
#extension GL_GOOGLE_include_directive : enable

// Total maximum supported number of lights
#include "maxlights.glslinc"

// Includes
#include "blinnphongutils.glslinc"
#include "utils.glslinc"

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
	vec3	ambient;						//< Ambient
	vec3	diffuse;						//< Diffuse
	vec3	specular;						//< Specular
	float	shininess;						//< Shininess
	float	alpha;							//< Alpha
} ubo;

// Fragment Input
in vec3 	passPosition;					//< Fragment position in world space
in vec3 	passNormal;						//< Fragment normal in world space

// Fragment Output
out vec4 out_Color;

uniform samplerCube environmentMap;

void main()
{
	// Material color
	BlinnPhongMaterial mtl = { ubo.ambient, ubo.diffuse, ubo.specular, ubo.shininess };

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

	// Final color output
	out_Color = vec4(color_result, ubo.alpha);
}
