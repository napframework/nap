/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local includes
#include "dofshader.h"

// External includes
#include <nap/core.h>
#include <renderadvancedservice.h>

// nap::DOFShader run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::DOFShader)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////
// DOFShader
//////////////////////////////////////////////////////////////////////////

namespace nap
{
	namespace shader
	{
		inline constexpr const char* dof = "dof";
	}

    DOFShader::DOFShader(Core& core) :
		Shader(core), mRenderAdvancedService(core.getService<RenderAdvancedService>())
	{ }

    bool DOFShader::init(utility::ErrorState& errorState)
    {
		if (!Shader::init(errorState))
			return false;

		const std::string display_name = shader::dof;
		const std::string vertex_shader_path = utility::joinPath({ "shaders", utility::appendFileExtension(display_name, "vert") });
		const std::string fragment_shader_path = utility::joinPath({ "shaders", utility::appendFileExtension(display_name, "frag") });

		// Read vert shader file
		std::string vert_source;
		if (!errorState.check(utility::readFileToString(vertex_shader_path, vert_source, errorState), "Unable to read shader file %s", vertex_shader_path.c_str()))
			return false;

		// Read frag shader file
		std::string frag_source;
		if (!errorState.check(utility::readFileToString(fragment_shader_path, frag_source, errorState), "Unable to read shader file %s", fragment_shader_path.c_str()))
			return false;

		// Search paths
		const auto search_paths = mRenderAdvancedService->getModule().getInformation().mDataSearchPaths;

		// Parse shader
		std::string shader_name = utility::getFileNameWithoutExtension(vertex_shader_path);
		if (!load(shader_name, search_paths, vert_source.data(), vert_source.size(), frag_source.data(), frag_source.size(), errorState))
			return false;

		return true;
    }
}
