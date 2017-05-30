#pragma once

#include "fileutils.h"
#include "logger.h"
#include "object.h"
#include "service.h"
#include "resource.h"
#include "rtti/jsonreader.h"
#include "rtti/rttireader.h"
#include <map>

namespace nap
{
	class DirectoryWatcher;

	/**
	 * Manager, holding resource data, capable of loading and real-time updating of content.
	 */
	class ResourceManagerService : public Service
	{
		RTTI_ENABLE(Service)
	public:

		ResourceManagerService();

		/**
		* Helper that calls loadFile without additional modified objects. See loadFile comments for a full description.
		*/
		bool loadFile(const std::string& filename, utility::ErrorState& errorState);

		/*
		* Loads a json file containing objects. When the objects are loaded, a comparison is performed against the objects that are already loaded. Only
		* the new objects and the objects that are different from the existing objects are loaded into the manager. The set of objects that is new
		* or changed then receives an init() call in the correct dependency order: if an object has a pointer to another object, the pointee is initted
		* first. In case an already existing object that wasn't in the file points to something that is changed, that object is recreated by 
		* cloning it and then calling init() on it. That also happens in the correct dependency order.
		*
		* Because there may be other objects pointing to objects that were read from json (which is only allowed through the ObjectPtr class), the updating
		* mechanism patches all those pointers before calling init(). 
		*
		* In case one of the init() calls fail, the previous state is completely restored by patching the pointers back and destroying objects that were read.
		* The client does not need to worry about handling such cases.
		* In case all init() calls succeed, any old objects are destructed (the cloned and the previously existing objects).
		*
		* @param filename: json file containing objects.
		* @param externalChangedFile: externally changed file that caused load of this file (like texture, shader etc)
		* @param errorState: if the function returns false, contains error information.
		*/
		bool loadFile(const std::string& filename, const std::string& externalChangedFile, utility::ErrorState& errorState);

		/**
		* Find a resource by object ID. Returns null if not found.
		*/
		Resource* findResource(const std::string& id);

		/**
		* Find a resource by object ID. Returns null if not found.
		*/
		template<class T>
		T* findResource(const std::string& id) { return rtti_cast<T>(findResource(id)); }

		/**
		* Creates a resource and adds it to the manager.
		*/
		Resource* createResource(const rtti::TypeInfo& type);

		/**
		* Creates a resource and adds it to the manager.
		*/
		template<typename T>
		T* createResource() { return rtti_cast<T>(createResource(RTTI_OF(T))); }

		/**
		* Function that runs the file monitor to check for changes. If changes are found in files that were loaded by the manager,
		* reloading and real-time updating of data takes place.
		*/
		void checkForFileChanges();

		/**
		* @return object capable of creating objects with custom construction parameters.
		*/
		rtti::Factory& getFactory();

	private:
		using ResourceMap = std::map<std::string, std::unique_ptr<Resource>>;
		using FileLinkMap = std::map<std::string, std::vector<std::string>>; // Map from target file to multiple source files
		using ObjectsToUpdate = std::unordered_map<std::string, std::unique_ptr<RTTIObject>>;

		void addResource(const std::string& id, std::unique_ptr<Resource> resource);
		void removeResource(const std::string& id);
		void addFileLink(const std::string& sourceFile, const std::string& targetFile);

		bool determineObjectsToInit(const ObjectsToUpdate& objectsToUpdate, const std::string& externalChangedFile, std::vector<std::string>& objectsToInit, utility::ErrorState& errorState);
		bool resolvePointers(ObjectsToUpdate& objectsToUpdate, const rtti::UnresolvedPointerList& unresolvedPointers, utility::ErrorState& errorState);
		bool initObjects(std::vector<std::string> objectsToInit, ObjectsToUpdate& objectsToUpdate, utility::ErrorState& errorState);
		template<typename MAP>
		void patchObjectPtrs(MAP& container);

	private:
		struct RollbackHelper
		{
		public:
			RollbackHelper(ResourceManagerService& service);
			~RollbackHelper();

			void clear();

		private:
			ResourceManagerService& mService;
			bool mPatchObjects = true;
		};

		ResourceMap							mResources;				// Holds all resources
		std::set<std::string>				mFilesToWatch;			// Files currently loaded, used for watching changes on the files
		FileLinkMap							mFileLinkMap;			// Map containing links from target to source file, for updating source files if the file monitor sees changes
		std::unique_ptr<DirectoryWatcher>	mDirectoryWatcher;		// File monitor, detects changes on files
	};


	/** 
	 * Traverses all pointers in ObjectPtrManager and, for each target, replaces the target with the one in the map that is passed.
	 * @param container The container holding an ID -> pointer mapping with the pointer to patch to.
	 */
	template<typename MAP>
	void ResourceManagerService::patchObjectPtrs(MAP& container)
	{
		ObjectPtrManager::ObjectPtrSet& object_ptrs = ObjectPtrManager::get().GetObjectPointers();

		for (ObjectPtrBase* ptr : object_ptrs)
		{
			RTTIObject* target = ptr->get();
			if (target == nullptr)
				continue;

			std::string& target_id = target->mID;
			MAP::iterator new_target = container.find(target_id);
			if (new_target != container.end())
				ptr->set(new_target->second.get());
		}
	}

}
