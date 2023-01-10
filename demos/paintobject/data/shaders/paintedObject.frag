/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#version 450 core

// vertex shader input  
in vec3 passUVs;						//< frag Uv's
in vec3 passNormal;						//< frag normal in world space
in vec3 passPosition;					//< frag world space position 
in mat4 passModelMatrix;				//< model matrix
in vec3 passVert;						// The vertex position

uniform nap
{
	mat4 projectionMatrix;
	mat4 viewMatrix;
	mat4 modelMatrix;
	mat4 normalMatrix;
	vec3 cameraPosition;
} mvp;

// uniform buffer inputs
uniform UBO
{
    vec3			LightPosition; 		//< light position in world space
	float 			LightIntensity; 	//< light intensity
} ubo;

// unfiorm sampler inputs 
uniform sampler2D	inPaintTexture;		//< Paint Texture
uniform sampler2D	inTexture;			//< Object Texture

// Light constants
const float			ambientIntensity = 0.5;					// Ambient light intensity
const float			shininess = 2.0;						// Specular angle shininess
const float			specularIntensity = 0.2;				// Amount of added specular

// output
out vec4 out_Color;

vec3 computeLightContribution(vec3 color)
{
		//calculate normal in world coordinates
    mat3 normal_matrix = transpose(inverse(mat3(passModelMatrix)));
    vec3 normal = normalize(normal_matrix * passNormal);
	
	//calculate the location of this fragment (pixel) in world coordinates
    vec3 frag_position = vec3(passModelMatrix * vec4(passVert, 1));

	//calculate the vector from this pixels surface to the light source
	vec3 surfaceToLight = normalize(ubo.LightPosition - frag_position);

	// calculate vector that defines the distance from camera to the surface
	vec3 surfaceToCamera = normalize(mvp.cameraPosition - frag_position);
	
	//diffuse
    float diffuseCoefficient = max(0.0, dot(normal, surfaceToLight));
	vec3 diffuse = diffuseCoefficient * color.rgb * ubo.LightIntensity;
    
	//specular
	vec3 specularColor = vec3(1.0,1.0,1.0);
	float specularCoefficient = 0.0;
    if(diffuseCoefficient > 0.0)
    {
        specularCoefficient = pow(max(0.0, dot(surfaceToCamera, reflect(-surfaceToLight, normal))), shininess);
    }
    vec3 specular = specularCoefficient * specularColor * ubo.LightIntensity * specularIntensity;

    // return combination
    return specular + diffuse;
}

void main() 
{
	// Use texture alpha to blend between two colors
	vec4 col = texture(inTexture, passUVs.xy);
	float alpha = col.a;

	// determine fragment color by the alpha and color of paint texture
	vec4 paint_col = texture(inPaintTexture, passUVs.xy);
	col = mix(col, paint_col, paint_col.a);
	col = vec4(col.rgb, alpha);

	// linear color (color before gamma correction)
    vec3 outColor = computeLightContribution(col.rgb);

    // Add ambient color
	vec3 ambient = col.rgb * ambientIntensity;
    outColor = outColor + ambient;
    out_Color = vec4(outColor, 1.0);
}
