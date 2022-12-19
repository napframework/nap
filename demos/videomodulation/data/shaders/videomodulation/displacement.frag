// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#version 450 core

// vertex shader input  
in vec3 passUVs;						//< frag Uv's
in vec3 passNormal;						//< normal object space
in vec3 passVert;						//< vertex world space coordinate
in mat4 passModelMatrix;				//< Used model matrix
in vec4 passColor;						//< Vertex color

// Point light structure
struct PointLight
{
	vec3		mPosition;
	float 		mIntensity;
};

// uniform inputs
uniform sampler2D	videoTextureFrag;

layout(binding = 0) uniform nap
{
	mat4 projectionMatrix;
	mat4 viewMatrix;
	mat4 modelMatrix;
	mat4 normalMatrix;
	vec3 cameraWorldPosition;
} mvp;

uniform UBO
{
	PointLight	light;				//< light
	vec3 		colorOne;			//< second color
	vec3 		colorTwo;			//< third color
	vec3 		colorThr;			//< fourth color
} ubo;

// output
out vec4 out_Color;

// constants
const float specularIntensity = 0.5;
const vec3  specularColor = vec3(1.0, 1.0, 1.0);
const float shininess = 4;
const float ambientIntensity = 0.45f;

void main() 
{
	//calculate the vector from this pixels surface to the light source
	vec3 surfaceToLight = normalize(ubo.light.mPosition - passVert);

	// calculate vector that defines the distance from camera to the surface
	vec3 surfaceToCamera = normalize(mvp.cameraWorldPosition - passVert);

	//calculate normal in world coordinates
    mat3 normal_matrix = transpose(inverse(mat3(passModelMatrix)));
    vec3 ws_normal = normalize(normal_matrix * passNormal);

	// Get texture rgba value
	vec3 tex_color = texture(videoTextureFrag, passUVs.xy).rgb;

	// Get both mix colors
	vec3 ver_color = passColor.rgb;
	float ver_greyscale = (ver_color.r + ver_color.g + ver_color.b) / 3.0;
	ver_color = mix(ubo.colorThr, ubo.colorOne, pow(ver_greyscale,2.0));
	vec3 ver_color_one = mix(ver_color, ubo.colorTwo, 0.5);
	vec3 ver_color_two = mix(ver_color, ubo.colorThr, 0.75);

	// Get textured color
	float greyscale = (tex_color.r + tex_color.g + tex_color.b) / 3.0; 

	// Mix vertex with texture color
	tex_color = mix(ver_color_one, ver_color_two, pow(greyscale,1.0));

	// Ambient color
	vec3 ambient = tex_color.rgb * ambientIntensity;

	// diffuse
    float diffuseCoefficient = max(0.0, dot(ws_normal, surfaceToLight));
	vec3 diffuse = diffuseCoefficient * tex_color.rgb * ubo.light.mIntensity;

	// Scale specular based on vert color (greyscale)
	float spec_intensity = min(specularIntensity + (pow(ver_greyscale,2) / 2.0),1.0);

	// Compute specularf
    float specularCoefficient = pow(max(0.0, dot(normalize(reflect(-surfaceToLight, ws_normal)), surfaceToCamera)), shininess);
    vec3 specular = specularCoefficient * specularColor * ubo.light.mIntensity * spec_intensity;

	//linear color (color before gamma correction)
    vec3 linearColor = diffuse + specular + ambient;

	// Set fragment color output to be texture color
	out_Color =  vec4(linearColor, 0.1);
}