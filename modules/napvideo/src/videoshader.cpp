/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local includes
#include "videoshader.h"
#include "renderservice.h"

// External includes
#include <nap/core.h>

// nap::VideoShader run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::VideoShader)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS


//////////////////////////////////////////////////////////////////////////
// VideoShader path literals
//////////////////////////////////////////////////////////////////////////

static inline char* videoVert = "shaders/video.vert";
static inline char* videoFrag = "shaders/video.frag";


//////////////////////////////////////////////////////////////////////////
// VideoShader
//////////////////////////////////////////////////////////////////////////

namespace nap
{
	VideoShader::VideoShader(Core& core) : Shader(core),
		mRenderService(core.getService<RenderService>()) { }


	bool VideoShader::init(utility::ErrorState& errorState)
	{
		std::string vertex_shader_path = mRenderService->getModule().findAsset(videoVert);
		if (!errorState.check(!vertex_shader_path.empty(), "%s: Unable to find video vertex shader %s", mRenderService->getModule().getName().c_str(), vertex_shader_path.c_str()))
			return false;

		std::string fragment_shader_path = mRenderService->getModule().findAsset(videoFrag);
		if (!errorState.check(!fragment_shader_path.empty(), "%s: Unable to find video fragment shader %s", mRenderService->getModule().getName().c_str(), fragment_shader_path.c_str()))
			return false;

		// Read vert shader file
		std::string vert_source;
		if (!errorState.check(utility::readFileToString(vertex_shader_path, vert_source, errorState), "Unable to read video vertex shader file"))
			return false;

		// Read frag shader file
		std::string frag_source;
		if (!errorState.check(utility::readFileToString(fragment_shader_path, frag_source, errorState), "Unable to read video fragment shader file"))
			return false;

		// Compile shader
		std::string shader_name = utility::getFileNameWithoutExtension(videoVert);
		return this->load(shader_name, vert_source.data(), vert_source.size(), frag_source.data(), frag_source.size(), errorState);
	}
}
