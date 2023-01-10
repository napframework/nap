// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#version 450 core

in vec3 pass_Uvs0;			// The global cover uvs
in vec3 pass_Normals;		// Normals
in mat4 pass_ModelMatrix;	// Matrix
in vec3 pass_Vert;			// The vertex position

out vec4 out_Color;

layout(binding = 0) uniform nap
{
	mat4 projectionMatrix;
	mat4 viewMatrix;
	mat4 modelMatrix;
	mat4 normalMatrix;
	vec3 cameraPosition;
} mvp;

uniform UBO
{
	vec3	lightPosition;		// World position of the light
	vec3	lightIntensity;		// Light intensity
	float	ambientIntensity;	// Ambient light intensity
	float	shininess;			// Specular angle shininess
	float	specularIntensity;	// Amount of added specular
	float	attenuationScale;	// Light Falloff
} ubo;

// Cover image input
uniform sampler2D	coverImage;				// Image to display

void main(void)
{
	// Fetch cover image color
	vec3 color = texture(coverImage, pass_Uvs0.xy).rgb;

	//calculate normal in world coordinates
    mat3 normal_matrix = transpose(inverse(mat3(pass_ModelMatrix)));
    vec3 normal = normalize(normal_matrix * pass_Normals);

	//calculate the location of this fragment (pixel) in world coordinates
    vec3 frag_position = vec3(pass_ModelMatrix * vec4(pass_Vert, 1));

	//calculate the vector from this pixels surface to the light source
	vec3 surface_to_light = normalize(ubo.lightPosition - frag_position);

	// calculate vector that defines the distance from camera to the surface
	vec3 surface_to_cam = normalize(mvp.cameraPosition - frag_position);

	// Ambient color
	vec3 ambient = color.rgb * ubo.lightIntensity * ubo.ambientIntensity;

	//diffuse
    float diffuse_co = max(0.0, dot(normal, surface_to_light));
	vec3 diffuse = diffuse_co * color.rgb * ubo.lightIntensity;
    
	//specular
	float spec_co = diffuse_co > 0.0 ? pow(max(0.0, dot(surface_to_cam, reflect(-surface_to_light, normal))), ubo.shininess) : 0.0;
    vec3 specular = spec_co * ubo.lightIntensity * ubo.specularIntensity;
    
	//attenuation based on light distance
    float distance_to_light = length(ubo.lightPosition - frag_position);
    float attenuation = 1.0 / (1.0 + ubo.attenuationScale * pow(distance_to_light, 2));

	//linear color (color before gamma correction)
    vec3 linear_color = ambient + attenuation*(diffuse + specular);

	//final color (after gamma correction)
	out_Color = vec4(linear_color, 1.0);
}