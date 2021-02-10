/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "gnomonshader.h"

// External Includes
#include <nap/core.h>

// nap::gnomonshader run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::GnomonShader)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////
// Gnomon Vertex Shader
//////////////////////////////////////////////////////////////////////////

static const char gnomonVertShader[] = R"glslang(
#version 450 core
uniform nap
{
	uniform mat4 projectionMatrix;
	uniform mat4 viewMatrix;
	uniform mat4 modelMatrix;
} mvp;

in vec3	in_Position;
in vec4 in_Color0;
out vec3 pass_Color;

void main(void)
{
    gl_Position = mvp.projectionMatrix * mvp.viewMatrix * mvp.modelMatrix * vec4(in_Position, 1.0);
	pass_Color = in_Color0.rgb;
}
)glslang";


//////////////////////////////////////////////////////////////////////////
// Gnomon Fragment Shader
//////////////////////////////////////////////////////////////////////////

static const char gnomonFragShader[] = R"glslang(
#version 450 core
in vec3 pass_Color;
out vec4 out_Color;
void main() 
{
    out_Color = vec4(pass_Color, 1.0);
}
)glslang";


namespace nap
{
	GnomonShader::GnomonShader(Core& core) : Shader(core)
	{ }


	bool GnomonShader::init(utility::ErrorState& errorState)
	{
		auto vert_size = sizeof(gnomonVertShader) - 1;
		auto frag_size = sizeof(gnomonFragShader) - 1;
		return load("GnomonShader", gnomonVertShader, vert_size, gnomonFragShader, frag_size, errorState);
	}
}