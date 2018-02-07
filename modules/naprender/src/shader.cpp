// Local Includes
#include "shader.h"
#include "material.h"

// External Includes
#include <utility/fileutils.h>
#include <nap/logger.h>

RTTI_BEGIN_CLASS(nap::Shader)
	RTTI_PROPERTY_FILELINK("mVertShader", &nap::Shader::mVertPath, nap::rtti::EPropertyMetaData::Required, nap::rtti::EPropertyFileType::VertShader)
	RTTI_PROPERTY_FILELINK("mFragShader", &nap::Shader::mFragPath, nap::rtti::EPropertyMetaData::Required, nap::rtti::EPropertyFileType::FragShader)
RTTI_END_CLASS

namespace nap
{
	// Store path and create display names
	bool Shader::init(utility::ErrorState& errorState)
	{
		if (!errorState.check(!mVertPath.empty(), "Vertex shader path not set"))
			return false;

		if (!errorState.check(!mFragPath.empty(), "Fragment shader path not set"))
			return false;

		// Set display name
		mDisplayName = utility::getFileNameWithoutExtension(mVertPath);

		mShader = std::make_unique<opengl::Shader>();

		// Initialize the shader
		mShader->init(mVertPath, mFragPath);
		if (!errorState.check(mShader->isLinked(), "unable to create shader program from vertex shader %s and fragment shader %s", mVertPath.c_str(), mFragPath.c_str()))
			return false;

		return true;
	}


	// Returns the associated opengl shader
	opengl::Shader& Shader::getShader()
	{
		assert(mShader != nullptr);
		return *mShader;
	}

}
