// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#version 450 core

in vec3 pass_Uvs0;			// The global cover uvs
in vec3 pass_Normals;		// Normals
in mat4 pass_ModelMatrix;	// Matrix
in vec3 pass_Vert;			// The vertex position

out vec4 out_Color;

uniform UBO
{
	uniform vec3		lightPosition;		// World position of the light
	uniform vec3		lightIntensity;		// Light intensity
	uniform float		ambientIntensity;	// Ambient light intensity
	uniform float		shininess;			// Specular angle shininess
	uniform float		specularIntensity;	// Amount of added specular
	uniform float		attenuationScale;	// Light Falloff
	uniform vec3		cameraLocation;		// World Space location of the camera
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
	vec3 surfaceToLight = normalize(ubo.lightPosition - frag_position);

	// calculate vector that defines the distance from camera to the surface
	vec3 cameraPosition = ubo.cameraLocation;
	vec3 surfaceToCamera = normalize(cameraPosition - frag_position);

	// Ambient color
	vec3 ambient = color.rgb * ubo.lightIntensity * ubo.ambientIntensity;

	//diffuse
    float diffuseCoefficient = max(0.0, dot(normal, surfaceToLight));
	vec3 diffuse = diffuseCoefficient * color.rgb * ubo.lightIntensity;
    
	//specular
	vec3 specularColor = vec3(1.0,1.0,1.0);
	float specularCoefficient = 0.0;
    if(diffuseCoefficient > 0.0)
        specularCoefficient = pow(max(0.0, dot(surfaceToCamera, reflect(-surfaceToLight, normal))), ubo.shininess);
    vec3 specular = specularCoefficient * specularColor * ubo.lightIntensity * ubo.specularIntensity;
    
	//attenuation based on light distance
    float distanceToLight = length(ubo.lightPosition - frag_position);
    float attenuation = 1.0 / (1.0 + ubo.attenuationScale * pow(distanceToLight, 2));

	//linear color (color before gamma correction)
    vec3 linearColor = ambient + attenuation*(diffuse + specular);

	//final color (after gamma correction)
	out_Color = vec4(linearColor, 1.0);
}