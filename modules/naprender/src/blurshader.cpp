/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local includes
#include "blurshader.h"
#include "renderservice.h"

// External includes
#include <nap/core.h>

RTTI_BEGIN_ENUM(nap::EBlurSamples)
	RTTI_ENUM_VALUE(nap::EBlurSamples::X5, "5x5"),
	RTTI_ENUM_VALUE(nap::EBlurSamples::X9, "9x9")
RTTI_END_ENUM

// nap::Blur5x5Shader run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::Blur5x5Shader)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

// nap::Blur9x9Shader run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::Blur9x9Shader)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS


//////////////////////////////////////////////////////////////////////////
// BlurShader
//////////////////////////////////////////////////////////////////////////

namespace nap
{
	namespace defaultshader
	{
		inline constexpr const char* gaussianblur5x5 = "gaussianblur";
		inline constexpr const char* gaussianblur9x9 = "gaussianblur9";
	}

    template <EBlurSamples KERNEL>
    BlurShader<KERNEL>::BlurShader(Core& core) : Shader(core), mRenderService(core.getService<RenderService>()) { }

    template <EBlurSamples KERNEL>
    bool BlurShader<KERNEL>::init(utility::ErrorState& errorState)
    {
		std::string shader_name;
		switch (KERNEL)
		{
		case nap::EBlurSamples::X5:
			shader_name = defaultshader::gaussianblur5x5;
			break;
		case nap::EBlurSamples::X9:
			shader_name = defaultshader::gaussianblur9x9;
			break;
		default:
			errorState.fail("Unknown blur sample type.");
			return false;
		}

		return loadDefault(shader_name, errorState);
    }

    // Explicit template instantiations
    template bool Blur5x5Shader::init(utility::ErrorState&);
    template bool Blur9x9Shader::init(utility::ErrorState&);
}
