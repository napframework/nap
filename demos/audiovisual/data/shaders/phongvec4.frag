// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#version 450 core

uniform nap
{
	mat4 projectionMatrix;
	mat4 viewMatrix;
	mat4 modelMatrix;
	mat4 normalMatrix;
	vec3 cameraPosition;
} mvp;

uniform UBO
{
	vec3 color;
	vec3 ambient;
	vec3 specular;
	vec3 fresnel;
	vec3 highlight;
	float alpha;
} ubo;

in vec3 passPosition;
in vec3 passNormal;
in vec2 passUV0;
in float passFresnel;
in float passFlux;

out vec4 out_Color;

const float shininess = 48.0;
const vec3 lightPosition = vec3(0.0, 0.0, 3.0);

void main() 
{
	// float intensity = smoothstep(1.0, 0.97, d);

	// Surface to camera normal
	vec3 surface_to_cam = normalize(mvp.cameraPosition - passPosition);

	// Calculate the vector from this pixels surface to the light source
	vec3 surface_to_light = normalize(lightPosition - passPosition);

	// Compute diffuse value
	vec3 normal = normalize(passNormal);
	float diffuse = max(0.0, dot(normal, surface_to_light));

	// compute specular value if diffuse coefficient is > 0.0
	float specular = 0.0;
	if (diffuse > 0.0)
	{
		vec3 halfway = normalize(-surface_to_light + surface_to_cam);  
		specular = pow(max(dot(normal, halfway), 0.0), shininess);
	}

	vec3 diffuse_color = mix(diffuse * ubo.color, ubo.ambient, 1.0-diffuse);
	vec3 specular_color = specular * ubo.specular;
	vec3 color = diffuse_color + specular_color;

	color = mix(color, ubo.fresnel, passFresnel);
	color = mix(color, ubo.highlight, smoothstep(0.975, 1.0, 1.0-passUV0.y));
	out_Color = vec4(color, ubo.alpha);
}
 