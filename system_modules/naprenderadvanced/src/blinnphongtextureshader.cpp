/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "blinnphongtextureshader.h"
#include "renderadvancedservice.h"

// NAP includes
#include <nap/core.h>
#include <renderservice.h>

// nap::BlinnPhongTextureShader run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::BlinnPhongTextureShader, "Default blinn-phong shader program with texture input, compatible with the 'render advanced' light system")
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	namespace shader
	{
		inline constexpr const char* blinnphongtexture = "blinnphongtexture";
		inline constexpr const char* blinnphongtexturenoshadow = "blinnphongtexturenoshadow";
	}

	BlinnPhongTextureShader::BlinnPhongTextureShader(Core& core) :
		Shader(core), mRenderAdvancedService(core.getService<RenderAdvancedService>())
	{ }


	bool BlinnPhongTextureShader::init(utility::ErrorState& errorState)
	{
        std::string shader_name = mRenderAdvancedService->isShadowMappingEnabled() ?
                shader::blinnphongtexture : shader::blinnphongtexturenoshadow;

        std::string relative_path = utility::joinPath({ "shaders", utility::appendFileExtension(shader_name, "vert") });
        const std::string vertex_shader_path = mRenderAdvancedService->getModule().findAsset(relative_path);
        if (!errorState.check(!vertex_shader_path.empty(), "%s: Unable to find %s vertex shader %s", mRenderAdvancedService->getModule().getName().c_str(), shader_name.c_str(), vertex_shader_path.c_str()))
            return false;

        relative_path = utility::joinPath({ "shaders", utility::appendFileExtension(shader_name, "frag") });
        const std::string fragment_shader_path = mRenderAdvancedService->getModule().findAsset(relative_path);
        if (!errorState.check(!vertex_shader_path.empty(), "%s: Unable to find %s fragment shader %s", mRenderAdvancedService->getModule().getName().c_str(), shader_name.c_str(), fragment_shader_path.c_str()))
            return false;

        // Read vert shader file
        std::string vert_source;
        if (!errorState.check(utility::readFileToString(vertex_shader_path, vert_source, errorState), "Unable to read %s vertex shader file", shader_name.c_str()))
            return false;

        // Read frag shader file
        std::string frag_source;
        if (!errorState.check(utility::readFileToString(fragment_shader_path, frag_source, errorState), "Unable to read %s fragment shader file", shader_name.c_str()))
            return false;

        // Copy data search paths
        const auto search_paths = mRenderAdvancedService->getModule().getInformation().mDataSearchPaths;

        // Compile shader
        if (!load(shader_name, search_paths, vert_source.data(), vert_source.size(), frag_source.data(), frag_source.size(), errorState))
            return false;

        return true;
	}
}
