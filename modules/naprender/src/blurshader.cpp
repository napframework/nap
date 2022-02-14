/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local includes
#include "blurshader.h"
#include "renderservice.h"

// External includes
#include <nap/core.h>
#include <utility/fileutils.h>


RTTI_BEGIN_ENUM(nap::EBlurSamples)
	RTTI_ENUM_VALUE(nap::EBlurSamples::X5, "5x5"),
	RTTI_ENUM_VALUE(nap::EBlurSamples::X9, "9x9")
RTTI_END_ENUM

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::Blur5x5Shader)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::Blur9x9Shader)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

namespace nap
{
    //////////////////////////////////////////////////////////////////////////
    // Shader path literals
    //////////////////////////////////////////////////////////////////////////

    inline constexpr const char* blurVert = "shaders/gaussianblur.vert";
    inline constexpr const char* blurFrag = "shaders/gaussianblur.frag";

    inline constexpr const char* blur9Vert = "shaders/gaussianblur9.vert";
    inline constexpr const char* blur9Frag = "shaders/gaussianblur9.frag";


    //////////////////////////////////////////////////////////////////////////
    // BlurShader
    //////////////////////////////////////////////////////////////////////////

    template <EBlurSamples KERNEL>
    BlurShader<KERNEL>::BlurShader(Core& core) : Shader(core), mRenderService(core.getService<RenderService>()) { }

    template <EBlurSamples KERNEL>
    bool BlurShader<KERNEL>::init(utility::ErrorState& errorState)
    {
        std::string vertex_shader_path = (KERNEL == EBlurSamples::X5) ?
                                         mRenderService->getModule().findAsset(blurVert) :
                                         mRenderService->getModule().findAsset(blur9Vert);

        if (!errorState.check(!vertex_shader_path.empty(), "%s: Unable to find blur vertex shader %s", mRenderService->getModule().getName().c_str(), vertex_shader_path.c_str()))
            return false;

        std::string fragment_shader_path = (KERNEL == EBlurSamples::X5) ?
                                           mRenderService->getModule().findAsset(blurFrag) :
                                           mRenderService->getModule().findAsset(blur9Frag);

        if (!errorState.check(!fragment_shader_path.empty(), "%s: Unable to find blur fragment shader %s", mRenderService->getModule().getName().c_str(), fragment_shader_path.c_str()))
            return false;

        // Read vert shader file
        std::string vert_source;
        if (!errorState.check(utility::readFileToString(vertex_shader_path, vert_source, errorState), "Unable to read blur vertex shader file"))
            return false;

        // Read frag shader file
        std::string frag_source;
        if (!errorState.check(utility::readFileToString(fragment_shader_path, frag_source, errorState), "Unable to read blur fragment shader file"))
            return false;

        // Compile shader
        std::string shader_name = utility::getFileNameWithoutExtension(blurVert);
        return this->load(shader_name, vert_source.data(), vert_source.size(), frag_source.data(), frag_source.size(), errorState);
    }

    // Explicit template instantiation
    template bool Blur5x5Shader::init(utility::ErrorState&);
    template bool Blur9x9Shader::init(utility::ErrorState&);
}
