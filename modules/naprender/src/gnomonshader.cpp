/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "gnomonshader.h"
#include "renderservice.h"

// External Includes
#include <nap/core.h>

// nap::GnomonShader run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::GnomonShader)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS


//////////////////////////////////////////////////////////////////////////
// GnomonShader
//////////////////////////////////////////////////////////////////////////

namespace nap
{
	namespace shader
	{
		inline constexpr const char* gnomon = "gnomon";
	}

	GnomonShader::GnomonShader(Core& core) : Shader(core),
		mRenderService(core.getService<RenderService>()) { }


	bool GnomonShader::init(utility::ErrorState& errorState)
	{
		return loadDefault(shader::gnomon, errorState);
	}
}
