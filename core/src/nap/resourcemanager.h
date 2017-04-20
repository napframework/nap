#pragma once

#include "fileutils.h"
#include "logger.h"
#include "object.h"
#include "service.h"
#include "resource.h"
#include <map>

namespace nap
{
	struct DirectoryWatcher;

	struct UnresolvedPointer
	{
		UnresolvedPointer(Object* object, const RTTI::Property& property, const std::string& targetID) :
			mObject(object),
			mProperty(property),
			mTargetID(targetID)
		{
		}

		Object* mObject;
		RTTI::Property mProperty;
		std::string mTargetID;
	};

	using ObjectList = std::vector<nap::Object*>;
	using UnresolvedPointerList = std::vector<UnresolvedPointer>;

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
		Resource* createResource(const RTTI::TypeInfo& type);

		template<typename T>
		T* createResource() { return rtti_cast<T>(createResource(RTTI_OF(T))); }

		void checkForFileChanges();

	private:
		using ExistingObjectMap = std::map<Object*, Object*>;
		void splitObjects(const ObjectList& sourceObjectList, ObjectList& targetObjectList, ExistingObjectMap& existingObjectMap, ObjectList& newObjectList);
		bool updateExistingObjects(const ExistingObjectMap& existingObjectMap, UnresolvedPointerList& unresolvedPointers, InitResult& initResult);
		void backupObjects(const ExistingObjectMap& objects, ExistingObjectMap& backups);
		void restoreObjects(ExistingObjectMap& objects, const ExistingObjectMap& backups);
		void rollback(ExistingObjectMap& existingObjects, const ExistingObjectMap& backupObjects, const ObjectList& newObjects);

		/**
		*/
		void addResource(const std::string& id, Resource* resource);

		void removeResource(const std::string& id);

	private:
		// Holds all currently loaded resources
		std::map<std::string, std::unique_ptr<Resource>> mResources;
		
		std::set<std::string> mFilesToWatch;
		DirectoryWatcher* mDirectoryWatcher;
	};

}

RTTI_DECLARE(nap::ResourceManagerService)
