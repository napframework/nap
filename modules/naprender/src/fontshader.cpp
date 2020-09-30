/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local includes
#include "fontshader.h"

// External includes
#include <nap/core.h>

// nap::VideoShader run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::FontShader)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////
// Font Vertex Shader
//////////////////////////////////////////////////////////////////////////

static const char fontVertShader[] = R"glslang(
#version 450 core
uniform nap
{
	uniform mat4 projectionMatrix;
	uniform mat4 viewMatrix;
	uniform mat4 modelMatrix;
} mvp;

in vec3	in_Position;
in vec3 in_UV0;
out vec3 passUVs;

void main(void)
{
    gl_Position = mvp.projectionMatrix * mvp.viewMatrix * mvp.modelMatrix * vec4(in_Position, 1.0);
	passUVs = in_UV0;
}
)glslang";


//////////////////////////////////////////////////////////////////////////
// Font Fragment Shader
//////////////////////////////////////////////////////////////////////////

static const char fontFragShader[] = R"glslang(
#version 450 core
in vec3 passUVs;
uniform sampler2D glyph;

uniform UBO
{
	uniform vec3 textColor;
} ubo;

out vec4 out_Color;
void main() 
{
	float alpha = texture(glyph, passUVs.xy).r;
    out_Color = vec4(ubo.textColor, alpha);
}
)glslang";


//////////////////////////////////////////////////////////////////////////
// VideoShader
//////////////////////////////////////////////////////////////////////////

namespace nap
{
	FontShader::FontShader(Core& core) : Shader(core) { }


	bool FontShader::init(utility::ErrorState& errorState)
	{
		// Number of characters = number of bytes minus null termination character of string literal.
		auto vert_size = sizeof(fontVertShader) - 1;
		auto frag_size = sizeof(fontFragShader) - 1;
		return load("FontShader", fontVertShader, vert_size, fontFragShader, frag_size, errorState);
	}
}