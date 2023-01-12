/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#version 450 core

// vertex shader input  
in vec3 passUVs;						//< frag Uv's
in vec3 passNormal;						//< frag normal in world space
in vec3 passPosition;					//< frag world space position 
in vec4 passColor;						//< frag color


// uniform buffer inputs
uniform UBO
{
	float inSoftness;			//< brush softness
	float inFalloff;			//< brush falloff
} ubo;

// output
out vec4 out_Color;

void main() 
{
	// center uv
	vec2 uv_center 			= vec2(0.5, 0.5);

	// get distance to center
	float dist				= distance(uv_center, passUVs.xy) * 2;

	// intensity of brush is determined by falloff
	float intensity			= dist - ubo.inFalloff;
	intensity 				= clamp(intensity, 0, 1);

	// intensity is toned down by softness
	intensity				= pow(1.0 - intensity, ubo.inSoftness);

	// when near edges, discard pixels, this is to prevent artifacts occuring on border of rendered texture
	if (passUVs.x <= 0.02 || passUVs.y <= 0.02 || passUVs.x >= 0.98 || passUVs.y >= 0.98)
    	intensity = 0;

   	// set the final outcolor
	vec4 out_col 			= vec4(intensity, intensity, intensity, 1);
	out_Color = out_col;
}
