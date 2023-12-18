/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "fadeshader.h"
#include "renderadvancedservice.h"

// Local includes
#include <nap/core.h>

// nap::FadeShader run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::FadeShader)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	namespace shader
	{
		inline constexpr const char* fade = "fade";
	}

	FadeShader::FadeShader(Core& core) :
		Shader(core), mRenderAdvancedService(core.getService<RenderAdvancedService>())
	{ }


	bool FadeShader::init(utility::ErrorState& errorState)
	{
		std::string relative_path = utility::joinPath({ "shaders", utility::appendFileExtension(shader::fade, "vert") });
		const std::string vertex_shader_path = mRenderAdvancedService->getModule().findAsset(relative_path);
		if (!errorState.check(!vertex_shader_path.empty(), "%s: Unable to find %s vertex shader %s", mRenderAdvancedService->getModule().getName().c_str(), shader::fade, vertex_shader_path.c_str()))
			return false;

		relative_path = utility::joinPath({ "shaders", utility::appendFileExtension(shader::fade, "frag") });
		const std::string fragment_shader_path = mRenderAdvancedService->getModule().findAsset(relative_path);
		if (!errorState.check(!vertex_shader_path.empty(), "%s: Unable to find %s fragment shader %s", mRenderAdvancedService->getModule().getName().c_str(), shader::fade, fragment_shader_path.c_str()))
			return false;

		// Read vert shader file
		std::string vert_source;
		if (!errorState.check(utility::readFileToString(vertex_shader_path, vert_source, errorState), "Unable to read %s vertex shader file", shader::fade))
			return false;

		// Read frag shader file
		std::string frag_source;
		if (!errorState.check(utility::readFileToString(fragment_shader_path, frag_source, errorState), "Unable to read %s fragment shader file", shader::fade))
			return false;

		// Copy data search paths
		const auto search_paths = mRenderAdvancedService->getModule().getInformation().mDataSearchPaths;

		// Compile shader
		return this->load(shader::fade, search_paths, vert_source.data(), vert_source.size(), frag_source.data(), frag_source.size(), errorState);
	}
}
