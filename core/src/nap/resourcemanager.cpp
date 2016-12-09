#include "resourcemanager.h"

namespace nap
{

	void ResourceManagerService::setAssetRoot(const std::string& dirname)
	{
		mResourcePath = dirname;
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
		if (mResourcePath.empty()) {
            Logger::fatal("No asset root set");
            return nullptr;
		}

        std::string assetRoot = getAbsolutePath(mResourcePath);

        if (!fileExists(assetRoot)) {
            Logger::fatal("Asset root does not exist: '%s'", assetRoot.c_str());
            return nullptr;
        }

		std::string filename = assetRoot + "/" + path;
		if (!fileExists(filename)) {
			Logger::fatal("File does not exist: '%s'", filename.c_str());
			return nullptr;
		}

		ResourceLoader* factory = getFactoryFor(filename);
		if (!factory) {
			Logger::fatal("Failed to find a factory for: %s", path.c_str());
			return nullptr;
		}

		std::unique_ptr<Resource> asset = std::move(factory->loadResource(filename));
		if (asset == nullptr)
			return nullptr;

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