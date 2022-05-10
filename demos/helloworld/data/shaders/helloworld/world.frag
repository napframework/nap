// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#version 450 core

// vertex shader input  
in vec3 passUVs;						//< frag Uv's
in vec3 passNormal;						//< frag normal in world space
in vec3 passPosition;					//< frag world space position 

// uniform buffer inputs
uniform UBO
{
	uniform vec3 cameraPosition;		//< Camera World Space Position
	uniform vec3 colorOne;				//< Mix Color One
	uniform vec3 colorTwo;				//< Mix Color Two
	uniform vec3 haloColor;				//< Halo Color
} ubo;

// unfiorm sampler inputs 
uniform sampler2D inWorldTexture;		//< World Texture

// output
out vec4 out_Color;

void main() 
{
	// Use texture alpha to blend between two colors
	float alpha = texture(inWorldTexture, passUVs.xy).r;
	vec3 world_color = mix(ubo.colorOne, ubo.colorTwo, alpha);

	// Calculate mesh to camera angle for halo effect
	vec3 cam_normal = normalize(ubo.cameraPosition - passPosition);

	// Dot product gives us the 'angle' between the surface and cam vector
	// The result is that normals pointing away from the camera at an angle of 90* are getting a higer value
	// Normals pointing towards the camera (directly) get a value of 0
	float cam_surface_dot = clamp(dot(normalize(passNormal), cam_normal),0.0,1.0);
	cam_surface_dot = clamp((1.0-cam_surface_dot) + 0.1, 0, 1);
	cam_surface_dot = pow(cam_surface_dot, 5.0);

	// Mix in the halo
	world_color = mix(world_color, ubo.haloColor, cam_surface_dot);

	// Set fragment color output
	out_Color =  vec4(world_color,1.0);
}
