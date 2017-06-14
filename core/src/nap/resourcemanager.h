#pragma once

#include "fileutils.h"
#include "logger.h"
#include "object.h"
#include "service.h"
#include "objectptr.h"
#include "rtti/jsonreader.h"
#include "rtti/rttireader.h"
#include <map>

namespace nap
{
	class DirectoryWatcher;
	class EntityInstance;
	class EntityResource;

	template<class ITERATORTYPE, class ELEMENTTYPE>
	class UniquePtrMapIterator
	{
	public:
		UniquePtrMapIterator(ITERATORTYPE pos) :
			mPos(pos)
		{
		}

		UniquePtrMapIterator operator++()
		{
			mPos++;
			return *this;
		}

		bool operator!=(const UniquePtrMapIterator& rhs)
		{
			return rhs.mPos != mPos;
		}

		ELEMENTTYPE operator*() const
		{
			return mPos->second.get();
		}

		ITERATORTYPE mPos;
	};

	template<class T, class ELEMENTTYPE>
	class UniquePtrMapWrapper
	{
	public:
		UniquePtrMapWrapper(T& map) :
			mMap(&map)
		{
		}

		using Iterator = UniquePtrMapIterator<typename T::iterator, ELEMENTTYPE>;
		using ConstIterator = UniquePtrMapIterator<typename T::const_iterator, const ELEMENTTYPE>;

		Iterator begin()			{ return Iterator(mMap->begin()); }
		Iterator end()				{ return Iterator(mMap->end()); }
		ConstIterator begin() const	{ return ConstIterator(mMap->begin()); }
		ConstIterator end() const	{ return ConstIterator(mMap->end()); }

	private:
		T* mMap;
	};

	/**
	 * Manager, holding all objects, capable of loading and real-time updating of content.
	 */
	class ResourceManagerService : public Service
	{
		RTTI_ENABLE(Service)
	public:
		using EntityByIDMap = std::unordered_map<std::string, std::unique_ptr<EntityInstance>>;

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
		* Find an object by object ID. Returns null if not found.
		*/
		const ObjectPtr<RTTIObject> findObject(const std::string& id);

		/**
		* Find an object by object ID. Returns null if not found.
		*/
		template<class T>
		const ObjectPtr<T> findObject(const std::string& id) { return ObjectPtr<T>(findObject(id)); }

		/**
		* Creates an object and adds it to the manager.
		*/
		const ObjectPtr<RTTIObject> createObject(const rtti::TypeInfo& type);

		/**
		* Instantiates an Entity.
		*/
		const ObjectPtr<EntityInstance> createEntity(const EntityResource& entityResource, const std::string& entityID, utility::ErrorState& errorState);

		/**
		* Creates an object and adds it to the manager.
		*/
		template<typename T>
		const ObjectPtr<T> createObject() { return ObjectPtr<T>(createObject(RTTI_OF(T))); }

		/**
		* Function that runs the file monitor to check for changes. If changes are found in files that were loaded by the manager,
		* reloading and real-time updating of data takes place.
		*/
		void checkForFileChanges();

		/**
		* @return object capable of creating objects with custom construction parameters.
		*/
		rtti::Factory& getFactory();

		const ObjectPtr<EntityInstance> findEntity(const std::string& inID) const;

		const EntityInstance& getRootEntity() const
		{
			return *mRootEntity;
		}

		EntityInstance& getRootEntity()
		{
			return *mRootEntity;
		}

		using EntityCollection = UniquePtrMapWrapper<EntityByIDMap, EntityInstance*>;
		EntityCollection getEntities() { return EntityCollection(mEntities); }

		virtual void initialized();

	private:
		using InstanceByIDMap = std::unordered_map<std::string, rtti::RTTIObject*>;
		using ObjectByIDMap = std::unordered_map<std::string, std::unique_ptr<RTTIObject>>;
		using FileLinkMap = std::unordered_map<std::string, std::vector<std::string>>; // Map from target file to multiple source files

		void addObject(const std::string& id, std::unique_ptr<RTTIObject> object);
		void removeObject(const std::string& id);
		void addFileLink(const std::string& sourceFile, const std::string& targetFile);

		bool determineObjectsToInit(const ObjectByIDMap& objectsToUpdate, const std::string& externalChangedFile, std::vector<std::string>& objectsToInit, utility::ErrorState& errorState);
		bool resolvePointers(ObjectByIDMap& objectsToUpdate, const rtti::UnresolvedPointerList& unresolvedPointers, utility::ErrorState& errorState);
		bool initObjects(std::vector<std::string> objectsToInit, ObjectByIDMap& objectsToUpdate, utility::ErrorState& errorState);
		bool initEntities(ObjectByIDMap& objectsToUpdate, utility::ErrorState& errorState);
		bool createEntities(const std::vector<const EntityResource*>& entityResources, EntityByIDMap& entityInstances, InstanceByIDMap& allNewInstances, utility::ErrorState& errorState);

		/** 
		* Traverses all pointers in ObjectPtrManager and, for each target, replaces the target with the one in the map that is passed.
		* @param container The container holding an ID -> pointer mapping with the pointer to patch to.
		*/
		template<class OBJECTSBYIDMAP> 
		void patchObjectPtrs(OBJECTSBYIDMAP& newTargetObjects);

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

		std::unique_ptr<EntityInstance>		mRootEntity;
		ObjectByIDMap						mObjects;				// Holds all objects
		EntityByIDMap						mEntities;
		EntityByIDMap						mEntitiesCreatedDuringInit;
		InstanceByIDMap						mInstancesCreatedDuringInit;
		std::set<std::string>				mFilesToWatch;			// Files currently loaded, used for watching changes on the files
		FileLinkMap							mFileLinkMap;			// Map containing links from target to source file, for updating source files if the file monitor sees changes
		std::unique_ptr<DirectoryWatcher>	mDirectoryWatcher;		// File monitor, detects changes on files
	};

	template<class OBJECTSBYIDMAP>
	void ResourceManagerService::patchObjectPtrs(OBJECTSBYIDMAP& newTargetObjects)
	{
		ObjectPtrManager::ObjectPtrSet& object_ptrs = ObjectPtrManager::get().GetObjectPointers();

		for (ObjectPtrBase* ptr : object_ptrs)
		{
			RTTIObject* target = ptr->get();
			if (target == nullptr)
				continue;

			std::string& target_id = target->mID;
			OBJECTSBYIDMAP::iterator new_target = newTargetObjects.find(target_id);
			if (new_target != newTargetObjects.end())
				ptr->set(&*(new_target->second));
		}
	}
}
