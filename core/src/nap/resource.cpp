#include "resource.h"
#include "resourcemanager.h"
#include "core.h"

namespace nap
{
	void ResourceLoader::addFileExtension(const std::string& ext)
	{
		if (supportsExtension(ext)) {
			Logger::warn("File extension was already added: %s", ext.c_str());
			return;
		}
		mFileExtensions.push_back(ext);
	}



	void ResourceLoader::setCore(Core& core)
	{
		mCore = &core;
	}



	std::string Resource::getResourcePath() const 
	{
        return mResourceManager->getResourcePath(*this);
    }


    bool ResourceLoader::supportsExtension(const std::string& extension) const
	{
		for (const std::string& ext : mFileExtensions)
			if (ext == extension)
				return true;
		return false;
	}


	bool ResourceLoader::canHandle(const std::string& assetPath) const
	{
		return supportsExtension(getFileExtension(assetPath));
	}
}

RTTI_DEFINE_BASE(nap::Resource)
RTTI_DEFINE_BASE(nap::ResourceLoader)
