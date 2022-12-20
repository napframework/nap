// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#version 450 core

uniform nap
{
	mat4 projectionMatrix;
	mat4 viewMatrix;
	mat4 modelMatrix;
	mat4 normalMatrix;
	vec3 cameraWorldPosition;
} mvp;

uniform FRAGUBO
{
	vec3		lightPosition;				// Light position
	float		lightIntensity;				// Light intensity
	vec3		diffuseColor;				// Color or the boid
	vec3		diffuseColorEx;				// Secondary color or the boid
	vec3		lightColor;					// Color of the light
	vec3		haloColor;					// Color of the halo/fresnel effect
	float		specularIntensity;			// Amount of added specular
	vec3		specularColor;				// Specular color
	float		shininess;					// Specular angle shininess
	float		ambientIntensity;			// Ambient of ambient light
	float		diffuseIntensity;			// Diffuse scaling factor
	float		mateColorRate;				// Maximum mate color percentage
	uint 		randomColor;				// Random color flag
} ubo;

in vec3 pass_Position;
in vec3 pass_Normal;

in float pass_Fresnel;
in float pass_Mates;
flat in uint pass_Id;

out vec4 out_Color;

// CONSTANTS
const float EPSILON 		= 0.00001;


// Converts HSV color vector 'c' to an RGB color
// Source: http://lolengine.net/blog/2013/07/27/rgb-to-hsv-in-glsl
vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}


float map(float value, float inMin, float inMax, float outMin, float outMax)
{
	return outMin + (outMax - outMin) * (value - inMin) / (inMax - inMin);
}


void main(void)
{
	// Diffuse color
	vec3 boid_hsv = vec3(mod(pass_Id, 360)/360.0, 1.0, 0.9);
	vec3 boid_randomrgb = hsv2rgb(boid_hsv);
	vec3 boid_diffuse = mix(ubo.diffuseColor, ubo.diffuseColorEx, map(pass_Mates, 0.05, ubo.mateColorRate, 0.0, 1.0));		
	vec3 boid_color = mix(boid_diffuse, boid_randomrgb, step(EPSILON, ubo.randomColor));

	// Surface to camera normal
	vec3 surface_to_cam = normalize(mvp.cameraWorldPosition - pass_Position);

	// Calculate the vector from this pixels surface to the light source
	vec3 surface_to_light = normalize(ubo.lightPosition - pass_Position);

	// Compute diffuse value
	float diffuse_coefficient = max(0.0, dot(pass_Normal, surface_to_light));

	// Compute specular value if coefficient is > 0.0
	vec3 halfway = normalize(surface_to_light + surface_to_cam);  
	float spec = pow(max(dot(pass_Normal, halfway), 0.0), ubo.shininess);
	float specular_coefficient = mix(0.0, spec, step(0.0, diffuse_coefficient));

	// Compute final diffuse contribution
	float diffuse = diffuse_coefficient * ubo.diffuseIntensity * ubo.lightIntensity;

	// Compute final specual contribution
	float specular = specular_coefficient * ubo.specularIntensity * ubo.lightIntensity;

	// Compute final ambient, diffuse and specular color values
	vec3 ambient_color = boid_color * ubo.ambientIntensity;
	vec3 diffuse_color = boid_color * ubo.lightColor * diffuse;
	vec3 specular_color = ubo.specularColor * specular;

	// Compute composite color value
	vec3 comp_color = diffuse_color + specular_color + ambient_color;
	comp_color = clamp(comp_color, vec3(0.0), vec3(1.0));

	// Compute final color value
	out_Color = vec4(mix(comp_color, ubo.haloColor, pass_Fresnel), 1.0); 
}
