/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local includes
#include "blurshader.h"
#include "renderservice.h"

// External includes
#include <nap/core.h>

// nap::VideoShader run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::BlurShader)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////
// BlurShader
//////////////////////////////////////////////////////////////////////////

namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// Shader path literals
	//////////////////////////////////////////////////////////////////////////

	inline constexpr const char* blurVert = "shaders/blur.vert";
	inline constexpr const char* blurFrag = "shaders/blur.frag";


	//////////////////////////////////////////////////////////////////////////
	// Blur Shader
	//////////////////////////////////////////////////////////////////////////

	BlurShader::BlurShader(Core& core) : Shader(core),
		mRenderService(core.getService<RenderService>()) { }


	bool BlurShader::init(utility::ErrorState& errorState)
	{
		std::string vertshader_path = mRenderService->getModule().findAsset(blurVert);
		if (!errorState.check(!vertshader_path.empty(), "%s: Unable to find blur vertex shader %s", mRenderService->getModule().getName().c_str(), vertshader_path.c_str()))
			return false;

		std::string fragshader_path = mRenderService->getModule().findAsset(blurFrag);
		if (!errorState.check(!vertshader_path.empty(), "%s: Unable to find blur vertex shader %s", mRenderService->getModule().getName().c_str(), fragshader_path.c_str()))
			return false;

		// Read vert shader file
		std::string vert_source;
		if (!errorState.check(utility::readFileToString(vertshader_path, vert_source, errorState), "Unable to read blur vertex shader file"))
			return false;

		// Read frag shader file
		std::string frag_source;
		if (!errorState.check(utility::readFileToString(fragshader_path, frag_source, errorState), "Unable to read blur fragment shader file"))
			return false;

		// Compile shader
		std::string shader_name = utility::getFileNameWithoutExtension(blurVert);
		return this->load(shader_name, vert_source.data(), vert_source.size(), frag_source.data(), frag_source.size(), errorState);
	}
}
