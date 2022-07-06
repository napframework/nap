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
	float		shininess;					// Specular angle shininess

	float		rainbowBlend;				// Rainbow blend value
} ubo;

in vec3 pass_Position;
in vec3 pass_Normal;
in float pass_Fresnel;
in float pass_Id;

out vec4 out_Color;


// Converts HSV color vector 'c' to an RGB color
// Source: http://lolengine.net/blog/2013/07/27/rgb-to-hsv-in-glsl
vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}


void main(void)
{
	// Particle color
	vec3 ptcl_hsv = vec3(mod(pass_Id, 360)/360.0, 0.85, 1.0);
	vec3 ptcl_randomrgb = hsv2rgb(ptcl_hsv);
	vec3 ptcl_diffuse = mix(ubo.diffuseColor, ptcl_randomrgb, ubo.rainbowBlend);

	// Surface to camera normal
	vec3 surface_to_cam = normalize(ubo.cameraLocation - pass_Position);

	// Calculate the vector from this pixels surface to the light source
	vec3 surface_to_light = normalize(ubo.lightPosition - pass_Position);

	// Compute diffuse value
	float diffuse = max(0.0, dot(pass_Normal, surface_to_light));

	// Compute specular value if coefficient is > 0.0
	vec3 halfway = normalize(surface_to_light + surface_to_cam);  
	float spec = pow(max(dot(pass_Normal, halfway), 0.0), ubo.shininess);
	float specular = mix(0.0, spec, step(0.0, diffuse)) * 0.333333;

	// Compute final diffuse and specular color values
	vec3 diffuse_color = ptcl_diffuse * ubo.lightColor * diffuse;
	vec3 specular_color = ubo.specularColor * specular;

	// Compute composite color value
	vec3 comp_color = diffuse_color + specular_color + ubo.ambientColor;
	comp_color = clamp(comp_color, 0.0, 1.0);

	// Compute final color value
	out_Color = vec4(mix(comp_color, ubo.haloColor, pass_Fresnel), 1.0); 
}
