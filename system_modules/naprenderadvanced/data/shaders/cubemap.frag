// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#version 450 core

// Extensions
#extension GL_GOOGLE_include_directive : enable

// Includes
#include "utils.glslinc"
#include "cubemap.glslinc"                               

// Uniforms
uniform UBO
{
	uint face;
} ubo;

// Fragment Input
in vec2 passUV;

// Fragment Output
out vec4 out_Color;

// Texture Sampler
uniform sampler2D equiTexture;


vec4 panoramaToCubeMap(uint face, vec2 uv)
{
	vec2 uv_flip = vec2(uv.x*2.0-1.0, (1.0-uv.y)*2.0-1.0);
	vec3 scan = uvToXYZ(face, uv_flip);
	vec3 dir = normalize(scan);
	vec2 src = dirToUV(dir);

	return texture(equiTexture, src);
}


void main()
{
	out_Color = panoramaToCubeMap(ubo.face, passUV);
}
