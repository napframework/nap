/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local includes
#include "textureshader.h"
#include "renderservice.h"

// External includes
#include <nap/core.h>

// nap::TextureShader run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::TextureShader, "Shader program that renders an object using a texture")
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS


//////////////////////////////////////////////////////////////////////////
// TextureShader
//////////////////////////////////////////////////////////////////////////

namespace nap
{
	namespace shader
	{
		inline constexpr const char* texture = "texture";
	}

	TextureShader::TextureShader(Core& core) : Shader(core),
		mRenderService(core.getService<RenderService>()) { }


	bool TextureShader::init(utility::ErrorState& errorState)
	{
		if (!Shader::init(errorState))
			return false;

		return loadDefault(shader::texture, errorState);
	}
}
