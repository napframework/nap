// Local includes
#include "constantshader.h"

// External includes
#include <nap/core.h>
#include <renderservice.h>

// nap::ConstantShader run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ConstantShader)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS


//////////////////////////////////////////////////////////////////////////
// FontShader path literals
//////////////////////////////////////////////////////////////////////////

static inline char* constantVert = "shaders/constant.vert";
static inline char* constantFrag = "shaders/constant.frag";


//////////////////////////////////////////////////////////////////////////
// ConstantShader
//////////////////////////////////////////////////////////////////////////

namespace nap
{
	ConstantShader::ConstantShader(Core& core) : Shader(core),
		mRenderService(core.getService<RenderService>()) { }


	bool ConstantShader::init(utility::ErrorState& errorState)
	{
		std::string vertex_shader_path = mRenderService->getModule().findAsset(constantVert);
		if (!errorState.check(!vertex_shader_path.empty(), "%s: Unable to find constant vertex shader %s", mRenderService->getModule().getName().c_str(), vertex_shader_path.c_str()))
			return false;

		std::string fragment_shader_path = mRenderService->getModule().findAsset(constantFrag);
		if (!errorState.check(!fragment_shader_path.empty(), "%s: Unable to find constant fragment shader %s", mRenderService->getModule().getName().c_str(), fragment_shader_path.c_str()))
			return false;

		// Read vert shader file
		std::string vert_source;
		if (!errorState.check(utility::readFileToString(vertex_shader_path, vert_source, errorState), "Unable to read constant vertex shader file"))
			return false;

		// Read frag shader file
		std::string frag_source;
		if (!errorState.check(utility::readFileToString(fragment_shader_path, frag_source, errorState), "Unable to read constant fragment shader file"))
			return false;

		// Compile shader
		std::string shader_name = utility::getFileNameWithoutExtension(constantVert);
		return this->load(shader_name, vert_source.data(), vert_source.size(), frag_source.data(), frag_source.size(), errorState);
	}
}
