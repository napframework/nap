// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#version 450 core

// vertex shader input 
in vec3 passNormals;							// Normals
in vec3 passVert;								// The vertex position

// NAP Uniforms
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
	vec3 surface_to_light = normalize(ubo.light.mPosition - frag_position);

	// calculate vector that defines the distance from camera to the surface
	vec3 surface_to_camera = normalize(mvp.cameraPosition - frag_position);
	
	//diffuse
    float diff_co = max(0.0, dot(normal, surface_to_light));
	vec3 diffuse = diff_co * color.rgb * ubo.light.mIntensity;
    
	//specular
	vec3 specular_color = vec3(1.0,1.0,1.0);
	float specular_co = diff_co > 0.0 ? pow(max(0.0, dot(surface_to_camera, reflect(-surface_to_light, normal))), shininess) : 0.0;
    vec3 specular = specular_co * specular_color * ubo.light.mIntensity * specularIntensity;

    // return combination
    return specular + diffuse;
}


void main() 
{        
	//linear color (color before gamma correction)
    vec3 color = computeLightContribution(ubo.meshColor);

    // Add ambient color
    color += ubo.meshColor.rgb * ambientIntensity;
    out_Color = vec4(color, 1.0);
}
