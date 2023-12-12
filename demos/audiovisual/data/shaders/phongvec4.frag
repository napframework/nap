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
	vec3 highlight;
	float alpha;
} ubo;

in vec3 passPosition;
flat in vec3 passNormal;
in vec2 passUV0;
in float passFresnel;
in float passFlux;

out vec4 out_Color;

const vec3 lightPosition = vec3(3.0, 3.0, 4.0);
const float shininess = 64.0;

void main() 
{
	float d = (1.0-passUV0.y);
	//float intensity = smoothstep(1.0, 0.97, d);
	float intensity = 1.0-smoothstep(0.01, 0.05, passPosition.y);
	
	vec3 diffuse = mix(ubo.color,  ubo.highlight, smoothstep(0.95, 1.0, d));
	//diffuse = min(mix(diffuse, ubo.highlight, passPosition.y*30.0), ubo.highlight);
	diffuse = mix(diffuse, ubo.highlight, passFlux*2.0);

	// Surface to camera normal
	vec3 surface_to_cam = normalize(mvp.cameraPosition - passPosition);

	// Calculate the vector from this pixels surface to the light source
	vec3 surface_to_light = normalize(lightPosition - passPosition);

	// Compute diffuse value
	vec3 normal = normalize(passNormal);
	float diffuse_coefficient = max(0.0, dot(normal, surface_to_light));

	// compute specular value if diffuse coefficient is > 0.0
	float specular = 0.0;
	if (diffuse_coefficient > 0.0)
	{
		vec3 halfway = normalize(-surface_to_light + surface_to_cam);  
		specular = pow(max(dot(normal, halfway), 0.0), shininess);
	}

	float light_intensity = 1.5;
	vec3 color = light_intensity * (diffuse_coefficient * diffuse) + light_intensity * specular;
	color = mix(color, vec3(1.0), passFresnel);

	out_Color = vec4(color, ubo.alpha);
}
 