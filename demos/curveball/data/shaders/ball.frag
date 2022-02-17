// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#version 450 core

// vertex shader input
in vec3 passUVs;						//< frag Uv's
in vec3 passNormal;						//< frag normal in object space
in vec3 passPosition;					//< frag position in object space
in mat4 passModelMatrix;				//< modelMatrix

// Point light structure
struct PointLight
{
	vec3		mPosition;
	float 		mIntensity;
};

// Uniform inputs
uniform UBO
{
	uniform vec3 		inCameraPosition;		//< Camera World Space Position
	uniform vec3		ballColor;
	uniform PointLight	light;
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
	mat3 normal_matrix = transpose(inverse(mat3(passModelMatrix)));
	vec3 ws_normal = normalize(normal * normal_matrix);

	// Calculate frag to world
	vec3 ws_position = vec3(passModelMatrix * vec4(position, 1.0));

	//calculate the vector from this pixels surface to the light source
	vec3 surfaceToLight = normalize(ubo.light.mPosition - ws_position);

	// calculate vector that defines the distance from camera to the surface
	vec3 surfaceToCamera = normalize(ubo.inCameraPosition - ws_position);

	// Ambient color
	vec3 ambient = color * ambientIntensity;

	// diffuse
    float diffuseCoefficient = max(0.0, dot(ws_normal, surfaceToLight));
	vec3 diffuse = diffuseCoefficient * color * ubo.light.mIntensity;

	// Scale specular based on vert color (greyscale)
	float spec_intensity = specularIntensity;

	// Compute specularf
    float specularCoefficient = pow(max(0.0, dot(normalize(reflect(-surfaceToLight, ws_normal)), surfaceToCamera)), shininess);
    vec3 specular = specularCoefficient * specularColor * ubo.light.mIntensity * spec_intensity;

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
