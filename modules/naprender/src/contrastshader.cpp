/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local includes
#include "contrastshader.h"
#include "renderservice.h"

// External includes
#include <nap/core.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ContrastShader)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS


namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// Shader path literals
	//////////////////////////////////////////////////////////////////////////

	inline constexpr const char* vertexShader = "shaders/contrast.vert";
	inline constexpr const char* fragmentShader = "shaders/contrast.frag";


	//////////////////////////////////////////////////////////////////////////
	// ContrastShader
	//////////////////////////////////////////////////////////////////////////

	ContrastShader::ContrastShader(Core& core) : Shader(core),
		mRenderService(core.getService<RenderService>()) { }


	bool ContrastShader::init(utility::ErrorState& errorState)
	{
		std::string vertshader_path = mRenderService->getModule().findAsset(vertexShader);
		if (!errorState.check(!vertshader_path.empty(), "%s: Unable to find blur vertex shader %s", mRenderService->getModule().getName().c_str(), vertshader_path.c_str()))
			return false;

		std::string fragshader_path = mRenderService->getModule().findAsset(fragmentShader);
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
		std::string shader_name = utility::getFileNameWithoutExtension(vertexShader);
		return this->load(shader_name, vert_source.data(), vert_source.size(), frag_source.data(), frag_source.size(), errorState);
	}
}
