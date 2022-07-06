// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#version 450 core

uniform FRAGUBO
{
	vec3		cameraLocation;				// World Space location of the camera

	vec3		lightPosition;				// Light position
	vec3		lightColor;					// Color of the light
	vec3		ambientColor;				// Color of the scene
	vec3		diffuseColor;				// Color of the surface
	vec3		specularColor;				// Specular color
	vec3		haloColor;					// Color of the halo/fresnel effect
	float		specularIntensity;			// Specular component intensity
	float		shininess;					// Specular angle shininess
} ubo;

// unfiorm sampler inputs 
uniform sampler2D colorTexture;				// Color Texture

in vec3 pass_Position;
in vec3 pass_UV0;
in vec3 pass_Normal;
in float pass_Fresnel;

out vec4 out_Color;


void main(void)
{
	// Use texture alpha to blend between two colors
	vec3 object_color = texture(colorTexture, pass_UV0.xy).rgb;

	// Surface to camera normal
	vec3 surface_to_cam = normalize(ubo.cameraLocation - pass_Position);

	// Calculate the vector from this pixels surface to the light source
	vec3 surface_to_light = normalize(ubo.lightPosition - pass_Position);

	// Compute diffuse value
	float diffuse = max(0.0, dot(pass_Normal, surface_to_light));

	// Compute specular value if coefficient is > 0.0
	vec3 halfway = normalize(surface_to_light + surface_to_cam);  
	float spec = pow(max(dot(pass_Normal, halfway), 0.0), ubo.shininess);
	float specular = mix(0.0, spec, step(0.0, diffuse)) * ubo.specularIntensity;

	// Compute final diffuse and specular color values
	vec3 diffuse_color = object_color * ubo.diffuseColor * ubo.lightColor * diffuse;
	vec3 specular_color = ubo.specularColor * specular;

	// Compute composite color value
	vec3 comp_color = diffuse_color + specular_color + ubo.ambientColor;
	comp_color = clamp(comp_color, 0.0, 1.0);

	// Compute final color value
	out_Color = vec4(mix(comp_color, ubo.haloColor, pass_Fresnel), 1.0); 
}
