// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#version 450 core

uniform UBO
{
	vec3		cameraLocation;				// World Space location of the camera
	vec3		lightPosition;				//
	float		lightIntensity;				//
	vec3		diffuseColor;				// Color or the mesh
	vec3		lightColor;					// Color of the light
	float		specularIntensity;			// Amount of added specular
	vec3		specularColor;				// Specular color
	float		shininess;					// Specular angle shininess
	float		ambientIntensity;			// Ambient of ambient light
	float		diffuseIntensity;			// Diffuse scaling factor
} ubo;

in vec3 pass_Position;
in vec3 pass_Normals;
//in vec3 pass_Uv;
flat in uint pass_Id;

out vec4 out_Color;


void main(void)
{
	uint boid_id = uint(pass_Id+0.1) % 2;
	vec3 boid_color = vec3(float(boid_id+1)*0.5, 0.0, 0.0);

	// Surface to camera normal     
    vec3 surface_to_cam_n = normalize(ubo.cameraLocation - pass_Position);
    vec3 surface_n = normalize(pass_Normals);
    
    float diffuse_v = 0.0;
    float specula_v = 0.0;

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

	// Compute final diffuse contribution
	diffuse_v = diffuse_v + (diffuse_coefficient * ubo.lightIntensity * ubo.lightIntensity * ubo.diffuseIntensity);

	// Compute final specual contribution
	specula_v = specula_v + (specula_coefficient * ubo.lightIntensity * ubo.lightIntensity * ubo.specularIntensity);

    // Compute final diffuse and specular color values
    vec3 diffuse_color = ubo.diffuseColor  * ubo.lightColor * diffuse_v;
    vec3 specula_color = ubo.specularColor * specula_v;

    // Get ambient color
	vec3 ambient_color = ubo.diffuseColor.rgb * ubo.ambientIntensity;

    // Compute final color value
    vec3 out_color = diffuse_color + specula_color + ambient_color;
    out_color = clamp(out_color, vec3(0.0, 0.0, 0.0), vec3(1.0, 1.0, 1.0));
    out_Color = vec4(out_color, 1.0); 
}
