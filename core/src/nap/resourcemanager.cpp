#include "resourcemanager.h"

namespace nap
{
	void ResourceLoader::addFileExtension(const std::string& ext)
	{
		if (hasExtension(ext)) {
			Logger::warn("File extension was already added: %s", ext.c_str());
			return;
		}
        mFileExtensions.push_back(ext);
	}


	bool ResourceLoader::hasExtension(const std::string& extension) const
	{
		for (const std::string& ext : mFileExtensions)
			if (ext == extension)
				return true;
		return false;
	}



	bool ResourceLoader::canHandle(const std::string& assetPath) const
	{
		return hasExtension(getFileExtension(assetPath));
	}


	void ResourceManagerService::setAssetRoot(const std::string& dirname)
	{
		mAssetRootDir = dirname;
		invalidate();
	}



	Resource* ResourceManagerService::getResource(const std::string& path)
	{
		const auto& it = mResources.find(path);
		if (it != mResources.end()) { // Resource was found
			return it->second.get();
		}
		return loadResource(path);
	}



	std::vector<ResourceLoader*> ResourceManagerService::getFactories()
	{
		std::vector<ResourceLoader*> factories;
		for (const RTTI::TypeInfo& factoryType : RTTI::TypeInfo::getRawTypes(RTTI::TypeInfo::get<ResourceLoader>())) {
			factories.push_back(getOrCreateFactory(factoryType));
		}
		return factories;
	}

    bool ResourceManagerService::canLoad(const std::string& path) {
        return getFactoryFor(path) != nullptr;
    }



    Resource* ResourceManagerService::loadResource(const std::string& path)
	{
        std::string filename = mAssetRootDir + "/" + path;
        if (!fileExists(filename)) {
            Logger::fatal("File does not exist: '%s'", filename.c_str());
            return nullptr;
        }

		ResourceLoader* factory = getFactoryFor(filename);
		if (!factory) {
			Logger::fatal("Failed to find a factory for: %s", path.c_str());
			return nullptr;
		}

		std::unique_ptr<Resource> asset;
		if (!factory->loadResource(filename, asset)) {
			return nullptr;
		}
        Resource* ptr = asset.get();
		mResources.emplace(path, std::move(asset));
		return ptr;
	}

	void ResourceManagerService::invalidate() { mResources.clear(); }


	ResourceLoader* ResourceManagerService::getFactoryFor(const std::string& path)
	{
		for (const auto factory : getFactories()) {
			if (factory->canHandle(path))
				return factory;
		}
		return nullptr;
	}


	ResourceLoader* ResourceManagerService::getOrCreateFactory(const RTTI::TypeInfo& factoryType)
	{
		assert(factoryType.isKindOf<ResourceLoader>());
		ResourceLoader* factory = getFactory(factoryType);
		if (!factory)
			factory = createFactory(factoryType);
		return factory;
	}



	ResourceLoader* ResourceManagerService::getFactory(const RTTI::TypeInfo& factoryType)
	{
		for (auto& factory : mFactories) {
			if (factory->getTypeInfo().isKindOf(factoryType))
				return factory.get();
		}
		return nullptr;
	}



	ResourceLoader* ResourceManagerService::createFactory(const RTTI::TypeInfo& factoryType)
	{
		auto factory = std::unique_ptr<ResourceLoader>(static_cast<ResourceLoader*>(factoryType.createInstance()));
		ResourceLoader* ptr = factory.get();
		ptr->setCore(getCore());
		mFactories.emplace_back(std::move(factory));
		return ptr;
	}
}