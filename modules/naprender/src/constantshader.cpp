/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local includes
#include "constantshader.h"
#include "renderservice.h"

// External includes
#include <nap/core.h>

// nap::ConstantShader run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ConstantShader)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS


//////////////////////////////////////////////////////////////////////////
// ConstantShader
//////////////////////////////////////////////////////////////////////////

namespace nap
{
	namespace defaultshader
	{
		inline constexpr const char* constant = "constant";
	}

	ConstantShader::ConstantShader(Core& core) : Shader(core),
		mRenderService(core.getService<RenderService>()) { }


	bool ConstantShader::init(utility::ErrorState& errorState)
	{
		return loadDefault(defaultshader::constant, errorState);
	}
}
