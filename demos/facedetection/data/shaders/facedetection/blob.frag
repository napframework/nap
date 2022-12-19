// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#version 450 core

// vertex shader input 
in vec3 passNormals;							// Normals
in vec3 passVert;								// The vertex position

// NAP Uniforms
layout(binding = 0) uniform nap
{
	mat4 projectionMatrix;
	mat4 viewMatrix;
	mat4 modelMatrix;
	mat4 normalMatrix;
	vec3 cameraWorldPosition;
} mvp;

// Point light structure
struct PointLight
{
	vec3		mPosition;
	vec3 		mIntensity;
};

// Uniform inputs
uniform UBO
{
	PointLight  light;                          // All ubo.lights in the scene
	vec3        meshColor;                      // Color or the mesh
} ubo;

// Light constants
const float		ambientIntensity = 0.5;			// Ambient light intensity
const float		shininess = 2.0;				// Specular angle shininess
const float		specularIntensity = 0.2;		// Amount of added specular

// output
out vec4 out_Color;


vec3 computeLightContribution(vec3 color)
{
	// calculate normal in world coordinates
    vec3 normal = normalize((mvp.normalMatrix * vec4(passNormals, 0.0)).xyz);
	
	//calculate the location of this fragment (pixel) in world coordinates
    vec3 frag_position = (mvp.modelMatrix * vec4(passVert, 1.0)).xyz;

	//calculate the vector from this pixels surface to the light source
	vec3 surfaceToLight = normalize(ubo.light.mPosition - frag_position);

	// calculate vector that defines the distance from camera to the surface
	vec3 surfaceToCamera = normalize(mvp.cameraWorldPosition - frag_position);
	
	//diffuse
    float diffuseCoefficient = max(0.0, dot(normal, surfaceToLight));
	vec3 diffuse = diffuseCoefficient * color.rgb * ubo.light.mIntensity;
    
	//specular
	vec3 specularColor = vec3(1.0,1.0,1.0);
	float specularCoefficient = 0.0;
    if(diffuseCoefficient > 0.0)
    {
        specularCoefficient = pow(max(0.0, dot(surfaceToCamera, reflect(-surfaceToLight, normal))), shininess);
    }
    vec3 specular = specularCoefficient * specularColor * ubo.light.mIntensity * specularIntensity;

    // return combination
    return specular + diffuse;
}


void main() 
{        
	//linear color (color before gamma correction)
    vec3 outColor = computeLightContribution(ubo.meshColor);

    // Add ambient color
	vec3 ambient = ubo.meshColor.rgb * ambientIntensity;
    outColor = outColor + ambient;
    out_Color = vec4(outColor, 1.0);
}
