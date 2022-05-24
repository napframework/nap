/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local includes
#include "textureshader.h"
#include "renderservice.h"

// External includes
#include <nap/core.h>

// nap::TextureShader run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::TextureShader)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS


//////////////////////////////////////////////////////////////////////////
// Texture Vertex Shader
//////////////////////////////////////////////////////////////////////////

static const char sTextureVert[] = R"glslang(
#version 450 core

uniform nap
{
	mat4 projectionMatrix;
	mat4 viewMatrix;
	mat4 modelMatrix;
} mvp;

in vec3	in_Position;
in vec3	in_UV0;

out vec3 pass_Uvs;

void main(void)
{
	// Calculate position
	gl_Position = mvp.projectionMatrix * mvp.viewMatrix * mvp.modelMatrix * vec4(in_Position, 1.0);

	// Pass color and uv's 
	pass_Uvs = in_UV0;
})glslang";


//////////////////////////////////////////////////////////////////////////
// Texture Fragment Shader
//////////////////////////////////////////////////////////////////////////

static const char sTextureFrag[] = R"glslang(
#version 450 core

uniform UBO
{
	uniform vec3 color;
	uniform float alpha;
} ubo;

in vec3 pass_Uvs;

out vec4 out_Color;

uniform sampler2D colorTexture;

void main(void)
{
	// Sample colorTexture
	vec4 texture_color = texture(colorTexture, pass_Uvs.xy);

	// Set output color
	out_Color = vec4(texture_color.rgb * ubo.color, texture_color.a * ubo.alpha);
})glslang";


//////////////////////////////////////////////////////////////////////////
// TextureShader
//////////////////////////////////////////////////////////////////////////

namespace nap
{
	namespace shader
	{
		constexpr const char* texture = "texture";
	}


	TextureShader::TextureShader(Core& core) : Shader(core),
		mRenderService(core.getService<RenderService>()) { }


	bool TextureShader::init(utility::ErrorState& errorState)
	{
		// Number of characters = number of bytes minus null termination character of string literal.
		auto vert_size = sizeof(sTextureVert) - 1;
		auto frag_size = sizeof(sTextureFrag) - 1;
		return load("textureshader", sTextureVert, vert_size, sTextureFrag, frag_size, errorState);
	}
}
