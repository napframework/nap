// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#version 450 core

// vertex shader input  
in vec3 passUVs;											//< frag Uv's
in mat4 passModelMatrix;									// Matrix
in vec3 passVert;											// The vertex position
in vec3 passNormals;										// Normals

// matrices
uniform nap
{
	uniform mat4 projectionMatrix;
	uniform mat4 viewMatrix;
	uniform mat4 modelMatrix;
	uniform mat4 normalMatrix;
	uniform vec3 cameraPosition;
} mvp;

// Blob detection structure
struct Blob
{
	vec3	mCenter;
	float	mSize;
};

// Point light structure
struct PointLight
{
	vec3		mPosition;
	vec3 		mIntensity;
};

// All uniform frag shader inputs
uniform UBO
{
	uniform PointLight 	light;								// All lights in the scene
	uniform Blob		blobs[20];							// All available blobs
	uniform int 		blobCount;							// Number of blobs
} ubo;

uniform sampler2D 	inTexture;								// Plane texture

// Light constants
const float			ambientIntensity = 0.1;					// Ambient light intensity
const float			shininess = 2.0;						// Specular angle shininess
const float			specularIntensity = 0.25;				// Amount of added specular

// output
out vec4 out_Color;


vec3 computeLightContribution(vec3 color)
{
		//calculate normal in world coordinates
    mat3 normal_matrix = transpose(inverse(mat3(passModelMatrix)));
    vec3 normal = normalize(normal_matrix * passNormals);
	
	//calculate the location of this fragment (pixel) in world coordinates
    vec3 frag_position = vec3(passModelMatrix * vec4(passVert, 1));

	//calculate the vector from this pixels surface to the light source
	vec3 surface_to_light = normalize(ubo.light.mPosition - frag_position);

	// calculate vector that defines the distance from camera to the surface
	vec3 surface_to_camera = normalize(mvp.cameraPosition - frag_position);
	
	//diffuse
    float diffuse_co = max(0.0, dot(normal, surface_to_light));
	vec3 diffuse = diffuse_co * color.rgb * ubo.light.mIntensity;
    
	//specular
	vec3 specularColor = vec3(1.0,1.0,1.0);
	float specular_co = diffuse_co > 0.0 ? pow(max(0.0, dot(surface_to_camera, reflect(-surface_to_light, normal))), shininess): 0.0;
    vec3 specular = specular_co * specularColor * ubo.light.mIntensity * specularIntensity;

    // return combination
    return specular + diffuse;
}


// Computes the distance to a certain blob
float getDistance(int blobID)
{
	//calculate the location of this fragment (pixel) in world coordinates
    vec3 frag_position = vec3(passModelMatrix * vec4(passVert, 1));
    vec3 blob_position = ubo.blobs[blobID].mCenter - vec3(0, ubo.blobs[blobID].mSize, 0.0);
    return length(blob_position - frag_position);
}


// maps value from min / max to output
float fit(float value, float min, float max, float outMin, float outMax)
{
    float v = clamp(value, min, max);
    float m = max - min;
    return (v - min) / (m) * (outMax - outMin) + outMin;
}


void main() 
{
	// Get texture rgba value
	vec3 tex_color = texture(inTexture, passUVs.xy).rgb;

	//linear color (color before gamma correction)
    vec3 light_color = computeLightContribution(tex_color);

    // Add ambient color
	vec3 ambient = tex_color * ambientIntensity;
	light_color = light_color + ambient;

	// Compute occlusion value
	float occ_value = 0.0;
	for(int i=0; i < ubo.blobCount; i++)
	{
		float blob_dist = getDistance(i);
		float lerp_v = fit(blob_dist, 0.0, ubo.blobs[i].mSize, 1.0, 0.0);
		lerp_v = smoothstep(0.0,1.0,pow(lerp_v, 0.85));
		occ_value = occ_value + lerp_v;
	}
	occ_value = clamp(occ_value, 0.0, 1.0);

	// Mix in occlusion
	light_color = mix(light_color,vec3(0.0,0.0,0.0), occ_value);

	// Set fragment color output to be texture color
	out_Color =  vec4(light_color, 1.0);
}