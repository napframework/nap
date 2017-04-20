#pragma once

#include "fileutils.h"
#include "logger.h"
#include "object.h"
#include "service.h"
#include "resource.h"

namespace nap
{
	struct DirectoryWatcher;

	/**
	 * An AssetManager deals with loading and caching resources.
	 * It provides a thin and easy to use interface to all AssetFactories.
	 */
	class ResourceManagerService : public Service
	{
		RTTI_ENABLE_DERIVED_FROM(Service)
	public:

		ResourceManagerService();

		bool loadFile(const std::string& filename, nap::InitResult& initResult);

		/**
		*/
		Resource* findResource(const std::string& id);

		template<class T>
		T* findResource(const std::string& id) { return rtti_cast<T>(findResource(id)); }

		/**
		*/
		void addResource(const std::string& id, Resource* resource);

		/**
		*/
		Resource* createResource(const RTTI::TypeInfo& type);

		template<typename T>
		T* createResource() { return rtti_cast<T>(createResource(RTTI_OF(T))); }

		void checkForFileChanges();

	private:
		// Holds all currently loaded resources
		std::map<std::string, std::unique_ptr<Resource>> mResources;
		
		std::set<std::string> mFilesToWatch;
		DirectoryWatcher* mDirectoryWatcher;
	};

}

RTTI_DECLARE(nap::ResourceManagerService)
