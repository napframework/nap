// Local Includes
#include "shaderresource.h"
#include "material.h"

// External Includes
#include <nap/fileutils.h>
#include <nap/logger.h>

RTTI_BEGIN_CLASS(nap::ShaderResource)
	RTTI_PROPERTY("mVertShader", &nap::ShaderResource::mVertPath)
	RTTI_PROPERTY("mFragShader", &nap::ShaderResource::mFragPath)
RTTI_END_CLASS

namespace nap
{
	// Display name derived from path
	const std::string nap::ShaderResource::getDisplayName() const
	{
		return mDisplayName;
	}


	// Store path and create display names
	bool ShaderResource::init(InitResult& initResult)
	{
		if (!initResult.check(!mVertPath.empty(), "Vertex shader path not set"))
			return false;

		if (!initResult.check(!mFragPath.empty(), "Fragment shader path not set"))
			return false;

		// Set display name
		mDisplayName = getFileNameWithoutExtension(mVertPath);

		// Initialize the shader
		mShader.init(mVertPath, mFragPath);
		if (!initResult.check(mShader.isLinked(), "unable to create shader program: %s", mVertPath.c_str(), mFragPath.c_str()))
			return false;

		return true;
	}


	// Returns the associated opengl shader
	opengl::Shader& ShaderResource::getShader()
	{
		return mShader;
	}

}

RTTI_DEFINE(nap::ShaderResource)
