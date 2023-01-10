// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#version 450 core

// vertex shader input
in vec3 passUVs;						//< frag Uv's
in vec3 passNormal;						//< frag normal in object space
in vec3 passPosition;					//< frag position in object space

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
	float 		mIntensity;
};

// Uniform inputs
uniform UBO
{
	vec3		ballColor;
	PointLight	light;
} ubo;

// output
out vec4 out_Color;

const float 	specularIntensity = 0.5;
const vec3  	specularColor = vec3(1.0, 1.0, 1.0);
const float 	shininess = 10;
const float 	ambientIntensity = 0.5f;

// Shades a color based on a light, incoming normal and position should be in object space
vec3 applyLight(vec3 color, vec3 normal, vec3 position)
{
	// Calculate normal to world
	vec3 ws_normal = normalize((vec4(passNormal, 0.0) * mvp.normalMatrix).xyz);

	// Calculate frag to world
	vec3 ws_position = vec3(mvp.modelMatrix * vec4(position, 1.0));

	//calculate the vector from this pixels surface to the light source
	vec3 surface_to_light = normalize(ubo.light.mPosition - ws_position);

	// calculate vector that defines the distance from camera to the surface
	vec3 surface_to_camera = normalize(mvp.cameraPosition - ws_position);

	// Ambient color
	vec3 ambient = color * ambientIntensity;

	// diffuse
    float diffuse_co = max(0.0, dot(ws_normal, surface_to_light));
	vec3 diffuse = diffuse_co * color * ubo.light.mIntensity;

	// Scale specular based on vert color (greyscale)
	float spec_intensity = specularIntensity;

	// Compute specularf
    float specular_co =  diffuse_co > 0.0 ? pow(max(0.0, dot(normalize(reflect(-surface_to_light, ws_normal)), surface_to_camera)), shininess) : 0.0;
    vec3 specular = specular_co * specularColor * ubo.light.mIntensity * spec_intensity;

	//linear color (color before gamma correction)
    return diffuse + specular + ambient;
}

void main()
{
	vec2 uvs = vec2(passUVs.x, passUVs.y);
    vec3 col = ubo.ballColor;
	vec3 output_color = applyLight(col, passNormal, passPosition);

	out_Color =  vec4(output_color.rgb, 1.0);
}
