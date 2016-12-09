#include "resource.h"
#include "resourcemanager.h"

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

    std::string Resource::getResourcePath() const {
        return mResourceManger->getResourcePath(*this);
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

RTTI_DEFINE(nap::Resource)
RTTI_DEFINE(nap::ResourceLoader)
