// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#version 450 core

// vertex shader input  
in vec3 passUVs;						//< frag Uv's
in vec3 passNormal;						//< frag normal in world space
in vec3 passPosition;					//< frag world space position 

// uniform inputs
uniform sampler2D inHeightmap;			//< World Texture

// All uniform fragment inputs
uniform FRAGUBO
{
	float blendValue;			//< height blend value
	vec3 lowerColor;			//< valley color
	vec3 upperColor;			//< peak color
	vec3 haloColor;				//< halo color
} fubo;

// output
out vec4 out_Color;


float fit(float value, float inMin, float inMax, float outMin, float outMax)
{
	float v = clamp(value, inMin, inMax);
	float m = inMax - inMin;
	if(m == 0.0)
		m = 0.00001;
	return (v - inMin) / (m) * (outMax - outMin) + outMin;
}


void main() 
{
	// Use texture alpha to blend between two colors
	float alpha = texture(inHeightmap, passUVs.xy).r;
	vec3 world_color = mix(fubo.lowerColor, fubo.upperColor, pow(alpha,3.0));

	// When there is pretty much no displacement, blend to lower color
	float blend_value = fit(pow(fubo.blendValue,0.75), 0.05, 0.9, 0.0, 1.0);
	world_color = mix(fubo.lowerColor, world_color, blend_value);

	// Calculate mesh to camera angle for halo effect
	vec3 cam_normal = normalize(mvp.cameraWorldPosition - passPosition);

	// Dot product gives us the 'angle' between the surface and cam vector
	// The result is that normals pointing away from the camera at an angle of 90* are getting a higer value
	// Normals pointing towards the camera (directly) get a value of 0
	float cam_surface_dot = clamp(dot(normalize(passNormal), cam_normal),0.0,1.0);
	cam_surface_dot = clamp((1.0-cam_surface_dot) + 0.1, 0, 1);
	cam_surface_dot = pow(cam_surface_dot, 2.0);

	// Mix in the halo
	world_color = mix(world_color, fubo.haloColor, cam_surface_dot);

	// Set fragment color output
	out_Color =  vec4(world_color,1.0);
}