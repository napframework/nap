// Local Includes
#include "shaderresource.h"
#include "material.h"

// External Includes
#include <nap/fileutils.h>
#include <nap/logger.h>

const std::string nap::ShaderResourceLoader::fragExtension = "frag";
const std::string nap::ShaderResourceLoader::vertExtension = "vert";

namespace nap
{
	// Display name derived from path
	const std::string& nap::ShaderResource::getDisplayName() const
	{
		return mDisplayName;
	}


	// Store path and create display names
	ShaderResource::ShaderResource(const std::string& vertPath, const std::string& fragPath) : mFragPath(fragPath), mVertPath(vertPath)
	{
		if (!hasExtension(mVertPath, ShaderResourceLoader::vertExtension))
			nap::Logger::warn("invalid vertex shader file, incorrect file extension, needs to be of type .vert");

		if (!hasExtension(mFragPath, ShaderResourceLoader::fragExtension))
			nap::Logger::warn("invalid frag shader file, incorrect file extension, needs to be of type .frag");

		// Set display name
		mDisplayName = getFileNameWithoutExtension(vertPath);
	}


	// Returns the associated opengl shader
	// Calling this will lazily load the shader if not done previously
	opengl::Shader& ShaderResource::getShader()
	{
		// Check if we tried to load the shader
		if (!mLoaded)
		{
			mShader.init(mVertPath, mFragPath);
			if (!mShader.isAllocated())
			{
				nap::Logger::warn("unable to create shader program: %s", mVertPath.c_str(), mFragPath.c_str());
			}
			mLoaded = true;
		}
		return mShader;
	}

	//////////////////////////////////////////////////////////////////////////

	// Add file extensions this loader can handle
	ShaderResourceLoader::ShaderResourceLoader()
	{
		addFileExtension("frag");
		addFileExtension("vert");
	}

	// Load shader based on resource path
	std::unique_ptr<Resource> ShaderResourceLoader::loadResource(const std::string& resourcePath) const
	{
		std::string path_without_ext = stripFileExtension(resourcePath);
		std::string frag_path = appendFileExtension(path_without_ext, fragExtension);
		std::string vert_path = appendFileExtension(path_without_ext, vertExtension);

		if (!fileExists(frag_path))
		{
			nap::Logger::warn("frag shader file does not exist: %s", frag_path.c_str());
			return nullptr;
		}

		if (!fileExists(vert_path))
		{
			nap::Logger::warn("vert shader file does not exist: %s", vert_path.c_str());
			return nullptr;
		}

		return std::make_unique<ShaderResource>(vert_path, frag_path);
	}

}

RTTI_DEFINE(nap::ShaderResource)
RTTI_DEFINE(nap::ShaderResourceLoader)