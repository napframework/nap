/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local includes
#include "coloradjustmentshader.h"
#include "renderservice.h"

// External includes
#include <nap/core.h>

// nap::ColorAdjustmentShader run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ColorAdjustmentShader)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS


//////////////////////////////////////////////////////////////////////////
// ColorAdjustmentShader
//////////////////////////////////////////////////////////////////////////

namespace nap
{
	namespace shader
	{
		inline constexpr const char* coloradjustment = "coloradjustment";
	}

	ColorAdjustmentShader::ColorAdjustmentShader(Core& core) : Shader(core),
		mRenderService(core.getService<RenderService>()) { }


	bool ColorAdjustmentShader::init(utility::ErrorState& errorState)
	{
		return loadDefault(shader::coloradjustment, errorState);
	}
}
