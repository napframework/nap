// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#version 450 core

in vec3 pass_Uvs0;			// The global vinyl uvs
in vec3 pass_Normals;		// Normals
in mat4 pass_ModelMatrix;	// Matrix
in vec3 pass_Vert;			// The vertex position
in vec3 pass_Tangent;		// The tangent
in vec3 pass_Bitangent;		// The bitangent

out vec4 out_Color;

// Fragment shader model matrix uniforms
uniform nap
{
	mat4 projectionMatrix;
	mat4 viewMatrix;
	mat4 modelMatrix;
	mat4 normalMatrix;
	vec3 cameraPosition;
} mvp;

// Fragment shader uniforms
uniform UBO
{
	vec3	recordColor;			// Color of the record
	vec3	lightPosition;			// Position of the light (world space)
	vec3	lightIntensity;			// Intensity of the light
	float	ambientIntensity;		// Ambient light intensity
	float	shininess;				// Specular angle ubo.shininess
	float	specularIntensity;		// Amount of added specular
	float	attenuationScale;		// Light Falloff
	float	grooveScale;			// Amount of groove highlight
	vec3	cameraLocation;			// World space location of the camera
} ubo;

// Samplers
uniform sampler2D	vinylLabel;			// Vinyl label texture
uniform sampler2D	vinylMask;			// Vinyl label mask texture
uniform sampler2D	grooveNormalMap;	// Normal map used for grooves

void main(void)
{
	// Get color of mask based on first uv set
	float mask_color = texture(vinylMask, pass_Uvs0.xy).r;

	// Label color
	vec3 label_color = texture(vinylLabel, pass_Uvs0.xy).rgb;
	vec3 color = mix(ubo.recordColor.rgb, label_color.rgb, mask_color);

	vec3 groove_color = texture(grooveNormalMap, pass_Uvs0.xy).rgb;
	vec3 groove_normal = (groove_color *2.0) - 1.0;
	groove_normal.rg = groove_normal.rg * ubo.grooveScale;

	// Convert normals to world
	vec3 normal_world = normalize((mvp.normalMatrix * vec4(pass_Normals,1.0)).xyz);
	vec3 tan_world = normalize((mvp.normalMatrix * vec4(pass_Tangent, 1.0)).xyz);
	vec3 bitan_world = cross(normal_world, tan_world) * 1.0;

	// Get normal transform matrix
	mat3 tbn_matrix = mat3(tan_world, bitan_world, normal_world);

	// Get final normal
    vec3 normal = normalize(tbn_matrix * groove_normal);

	//calculate the location of this fragment (pixel) in world coordinates
    vec3 frag_position = vec3(pass_ModelMatrix * vec4(pass_Vert, 1));

	//calculate the vector that defines the direction of the light to the surface
	vec3 surface_to_light = normalize(ubo.lightPosition - frag_position);

	// calculate vector that defines the direction of camera to the surface
	vec3 surface_to_cam = normalize(mvp.cameraPosition - frag_position);

	// Ambient color
	vec3 ambient = color.rgb * ubo.lightIntensity * ubo.ambientIntensity;

	//diffuse
    float diffuse_co = max(0.0, dot(normal, surface_to_light));
	vec3 diffuse = diffuse_co * color.rgb * ubo.lightIntensity;

	//specular
	vec3 specular_color = vec3(1.0,1.0,1.0);
	float specular_co = diffuse_co > 0.0 ? pow(max(0.0, dot(surface_to_cam, reflect(-surface_to_light, normal))), ubo.shininess) : 0.0;
    vec3 specular = specular_co * ubo.lightIntensity * ubo.specularIntensity;

	//attenuation based on light distance
    float distance_to_light = length(ubo.lightPosition - frag_position);
    float attenuation = 1.0 / (1.0 + ubo.attenuationScale * pow(distance_to_light, 2));

	//linear color (color before gamma correction)
    vec3 linear_color = ambient + attenuation*(diffuse + specular);

	//final color (after gamma correction)
	out_Color = vec4(linear_color, 1.0);
}
