// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#version 450 core

// vertex shader input 
in vec3 passNormals;									// Normal
in vec3 passPosition;									// The vertex position in object space

uniform nap
{
	mat4 projectionMatrix;
	mat4 viewMatrix;
	mat4 modelMatrix;
	mat4 normalMatrix;
	vec3 cameraPosition;
} mvp;

// Point light structure
struct PointLight
{
	vec3		mPosition;
	vec3 		mIntensity;
};

uniform UBO
{
	PointLight	lights[2];								// All lights in the scene
	vec3 		meshColor;								// Color or the mesh
} ubo;

// Light Uniforms
const float		ambientIntensity = 0.3;					// Ambient light intensity
const float		shininess = 4.0;						// Specular angle shininess
const float		specularIntensity = 0.5;				// Amount of added specular

// output
out vec4 out_Color;


vec3 computeLightContribution(int lightIndex, vec3 color)
{
	// Calculate vertex world position
	vec3 frag_pos = (mvp.modelMatrix * vec4(passPosition, 1.0)).xyz;

	// Rotate normal based on model matrix and set
	vec3 frag_normal = normalize((mvp.normalMatrix * vec4(passNormals, 0.0)).xyz); 

	// Calculate the vector from this pixels surface to the light source
	vec3 surfaceToLight = normalize(ubo.lights[lightIndex].mPosition - frag_pos);

	// Calculate vector that defines the distance from camera to the surface
	vec3 surfaceToCamera = normalize(mvp.cameraPosition - frag_pos);
	
	// Diffuse
    float diffuseCoefficient = max(0.0, dot(frag_normal, surfaceToLight));
	vec3 diffuse = diffuseCoefficient * color.rgb * ubo.lights[lightIndex].mIntensity;
    
	// Specular
	vec3 specularColor = vec3(1.0);
	float specularCoefficient = 0.0;
    if(diffuseCoefficient > 0.0)
        specularCoefficient = pow(max(0.0, dot(surfaceToCamera, reflect(-surfaceToLight, frag_normal))), shininess);
    vec3 specular = specularCoefficient * specularColor * ubo.lights[lightIndex].mIntensity * specularIntensity;

    // Return combination
    return specular + diffuse;
}


void main() 
{        
	// Linear color (color before gamma correction)
    vec3 outColor = vec3(0.0);
    for(int i=0; i<ubo.lights.length(); i++)
    {
    	outColor = outColor + computeLightContribution(i, ubo.meshColor);
    }

    // Add ambient color
	vec3 ambient = ubo.meshColor.rgb * ambientIntensity;
    outColor = outColor + ambient;
    out_Color = vec4(outColor, 1.0);
}
