// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#version 450 core

uniform UBO
{
	vec3		cameraLocation;				// World Space location of the camera
	vec3		lightPosition;				// Light position
	float		lightIntensity;				// Light intensity
	vec3		diffuseColor;				// Color or the mesh
	vec3		lightColor;					// Color of the light
	float		specularIntensity;			// Amount of added specular
	vec3		specularColor;				// Specular color
	float		shininess;					// Specular angle shininess
	float		ambientIntensity;			// Ambient of ambient light
	float		diffuseIntensity;			// Diffuse scaling factor
	float		mateColorRate;				// Maximum mate color percentage
	uint 		randomColor;				// Random color flag
} ubo;

in vec3 pass_Position;
in vec3 pass_Normals;

in float pass_Speed;
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
	// Theme color
	vec3 boid_color = ubo.diffuseColor;

	// Rainbow
	vec3 boid_hsv = vec3(float(mod(pass_Id, 360)/360.0), 1.0, 0.9);
	if (ubo.randomColor > EPSILON)
	{	
		boid_color = hsv2rgb(boid_hsv);
	}
	else
	{
		boid_color = mix(boid_color, hsv2rgb(boid_hsv), map(pow(pass_Mates, 0.5), 0.0, ubo.mateColorRate, 0.0, 1.0));
	}

	// Surface to camera normal     
	vec3 surface_to_cam_n = normalize(ubo.cameraLocation - pass_Position);
	vec3 surface_n = normalize(pass_Normals);

	// Calculate the vector from this pixels surface to the light source
	vec3 surface_to_light = ubo.lightPosition - pass_Position;
	vec3 surface_to_light_n = normalize(surface_to_light);

	// Compute diffuse value
	float diffuse_coefficient = max(0.0, dot(surface_n, surface_to_light_n));

	// Compute specular value if coefficient is > 0.0
	float specula_coefficient = 0.0;
	if (diffuse_coefficient > 0.0)
	{
		vec3 halfway = normalize(surface_to_light_n + surface_to_cam_n);  
		specula_coefficient = pow(max(dot(surface_n, halfway), 0.0), ubo.shininess);
	}

	float diffuse_v = 0.0;
	float specula_v = 0.0;

	// Compute final diffuse contribution
	diffuse_v += diffuse_coefficient * ubo.lightIntensity * ubo.lightIntensity * ubo.diffuseIntensity;

	// Compute final specual contribution
	specula_v += specula_coefficient * ubo.lightIntensity * ubo.lightIntensity * ubo.specularIntensity;

	// Compute final diffuse and specular color values
	vec3 diffuse_color = boid_color * ubo.lightColor * diffuse_v;
	vec3 specular_color = ubo.specularColor * specula_v;

	// Get ambient color
	vec3 ambient_color = boid_color * ubo.ambientIntensity;

	// Compute composite color value
	vec3 comp_color = diffuse_color + specular_color + ambient_color;
	comp_color = clamp(comp_color, vec3(0.0), vec3(1.0));

	// Compute final color value
	out_Color = vec4(mix(comp_color, vec3(1.0), pass_Fresnel), 1.0); 
}
