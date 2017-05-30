// Local Includes
#include "shaderresource.h"
#include "material.h"

// External Includes
#include <nap/fileutils.h>
#include <nap/logger.h>

RTTI_BEGIN_CLASS(nap::ShaderResource)
	RTTI_PROPERTY("mVertShader", &nap::ShaderResource::mVertPath, nap::rtti::EPropertyMetaData::FileLink | nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("mFragShader", &nap::ShaderResource::mFragPath, nap::rtti::EPropertyMetaData::FileLink | nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

namespace nap
{
	// Display name derived from path
	const std::string nap::ShaderResource::getDisplayName() const
	{
		return mDisplayName;
	}


	// Store path and create display names
	bool ShaderResource::init(utility::ErrorState& errorState)
	{
		if (!errorState.check(!mVertPath.empty(), "Vertex shader path not set"))
			return false;

		if (!errorState.check(!mFragPath.empty(), "Fragment shader path not set"))
			return false;

		// Set display name
		mDisplayName = getFileNameWithoutExtension(mVertPath);

		mShader = std::make_unique<opengl::Shader>();

		// Initialize the shader
		mShader->init(mVertPath, mFragPath);
		if (!errorState.check(mShader->isLinked(), "unable to create shader program: %s", mVertPath.c_str(), mFragPath.c_str()))
			return false;

		return true;
	}


	// Returns the associated opengl shader
	opengl::Shader& ShaderResource::getShader()
	{
		assert(mShader != nullptr);
		return *mShader;
	}

}

RTTI_DEFINE(nap::ShaderResource)
