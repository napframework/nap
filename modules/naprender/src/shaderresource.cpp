// Local Includes
#include "shaderresource.h"

// External Includes
#include <nap/stringutils.h>

namespace nap
{
	const std::string& nap::ShaderResource::getDisplayName() const
	{
		return mDisplayName;
	}


	Object* nap::ShaderResource::createInstance(Core& core, Object& parent)
	{
		assert(false);
		return nullptr;
	}


	// Store path and create display names
	ShaderResource::ShaderResource(const std::string& vertPath, const std::string& fragPath) : mFragPath(fragPath), mVertPath(vertPath)
	{
		if (!hasExtension(vertPath, "vert"))
			nap::Logger::warn("invalid vertex shader file, incorrect file extension, needs to be of type .vert");

		if (!hasExtension(fragPath, "frag"))
			nap::Logger::warn("invalid frag shader file, incorrect file extension, needs to be of type .frag");

		// Set display name
		mDisplayName = getFileNameWithoutExtension(vertPath);
	}
}

RTTI_DEFINE(nap::ShaderResource)