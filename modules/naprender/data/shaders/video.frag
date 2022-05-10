// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#version 450 core

uniform sampler2D yTexture;
uniform sampler2D uTexture;
uniform sampler2D vTexture;

in vec3 pass_Uvs;

const vec3 R_cf = vec3(1.164383,  0.000000,  1.596027);
const vec3 G_cf = vec3(1.164383, -0.391762, -0.812968);
const vec3 B_cf = vec3(1.164383,  2.017232,  0.000000);

const vec3 offset = vec3(-0.0625, -0.5, -0.5);

out vec4 out_Color;

void main() 
{
	float y = texture(yTexture, vec2(pass_Uvs.x, 1.0-pass_Uvs.y)).r;
	float u = texture(uTexture, vec2(pass_Uvs.x, 1.0-pass_Uvs.y)).r;
	float v = texture(vTexture, vec2(pass_Uvs.x, 1.0-pass_Uvs.y)).r;

	vec3 yuv = vec3(y,u,v);
	yuv += offset;

	out_Color = vec4(dot(yuv, R_cf), dot(yuv, G_cf), dot(yuv, B_cf), 1.0);
}
