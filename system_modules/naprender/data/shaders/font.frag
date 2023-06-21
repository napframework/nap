// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#version 450 core

uniform UBO
{
	uniform vec3 textColor;
} ubo;

uniform sampler2D glyph;

in vec3 passUVs;
out vec4 out_Color;

void main() 
{
	float alpha = texture(glyph, passUVs.xy).r;
    out_Color = vec4(ubo.textColor, alpha);
}
