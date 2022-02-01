/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "gnomonshader.h"
#include "renderservice.h"

// External Includes
#include <nap/core.h>
#include <renderservice.h>

// nap::gnomonshader run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::GnomonShader)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS


//////////////////////////////////////////////////////////////////////////
// GnomonShader path literals
//////////////////////////////////////////////////////////////////////////

static inline char* gnomonVert = "shaders/gnomon.vert";
static inline char* gnomonFrag = "shaders/gnomon.frag";


//////////////////////////////////////////////////////////////////////////
// GnomonShader
//////////////////////////////////////////////////////////////////////////

namespace nap
{
	GnomonShader::GnomonShader(Core& core) : Shader(core),
		mRenderService(core.getService<RenderService>()) { }


	bool GnomonShader::init(utility::ErrorState& errorState)
	{
		std::string vertex_shader_path = mRenderService->getModule().findAsset(gnomonVert);
		if (!errorState.check(!vertex_shader_path.empty(), "%s: Unable to find gnomon vertex shader %s", mRenderService->getModule().getName().c_str(), vertex_shader_path.c_str()))
			return false;

		std::string fragment_shader_path = mRenderService->getModule().findAsset(gnomonFrag);
		if (!errorState.check(!fragment_shader_path.empty(), "%s: Unable to find gnomon fragment shader %s", mRenderService->getModule().getName().c_str(), fragment_shader_path.c_str()))
			return false;

		// Read vert shader file
		std::string vert_source;
		if (!errorState.check(utility::readFileToString(vertex_shader_path, vert_source, errorState), "Unable to read gnomon vertex shader file"))
			return false;

		// Read frag shader file
		std::string frag_source;
		if (!errorState.check(utility::readFileToString(fragment_shader_path, frag_source, errorState), "Unable to read gnomon fragment shader file"))
			return false;

		// Compile shader
		std::string shader_name = utility::getFileNameWithoutExtension(gnomonVert);
		return this->load(shader_name, vert_source.data(), vert_source.size(), frag_source.data(), frag_source.size(), errorState);
	}
}
