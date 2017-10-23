#pragma once

// Local Includes
#include "rtti/rtti.h"
#include "objectptr.h"
#include "utility/dllexport.h"
#include "utility/uniqueptrmapiterator.h"
#include "directorywatcher.h"
#include "entity.h"
#include "component.h"
#include "configure.h"

// External Includes
#include <rtti/unresolvedpointer.h>
#include <rtti/factory.h>
#include <map>

namespace nap
{	
	class Core;
	class RTTIObjectGraphItem;
	template<typename ITEM> class ObjectGraph;
	using RTTIObjectGraph = ObjectGraph<RTTIObjectGraphItem>;

	/**
	 * Manager, owner of all objects, capable of loading and real-time updating of content.
	 */
	class NAPAPI ResourceManagerService
	{
		RTTI_ENABLE()
	public:
		using EntityByIDMap = std::unordered_map<std::string, std::unique_ptr<EntityInstance>>;
		using EntityIterator = utility::UniquePtrMapWrapper<EntityByIDMap, EntityInstance*>;

		ResourceManagerService(nap::Core& core);

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
		const ObjectPtr<rtti::RTTIObject> findObject(const std::string& id);

		/**
		* Find an object by object ID. Returns null if not found.
		*/
		template<class T>
		const ObjectPtr<T> findObject(const std::string& id) { return ObjectPtr<T>(findObject(id)); }

		/**
		* Creates an object and adds it to the manager.
		*/
		const ObjectPtr<rtti::RTTIObject> createObject(const rtti::TypeInfo& type);

		/**
		* Instantiates an Entity.
		*/
		const ObjectPtr<EntityInstance> createEntity(const Entity& Entity, EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState);

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

		/**
		* @return EntityInstance.
		*/
		const ObjectPtr<EntityInstance> findEntity(const std::string& inID) const;

		/**
		* @return The root entity as created by the system, which is the root parent of all entities.
		*/
		const EntityInstance& getRootEntity() const
		{
			return *mRootEntity;
		}

		/**
		* @return The root entity as created by the system, which is the root parent of all entities.
		*/
		EntityInstance& getRootEntity()
		{
			return *mRootEntity;
		}

		/**
		* @return Iterator to all entities in the system.
		*/
		EntityIterator getEntities() { return EntityIterator(mEntities); }

		/**
		 * Forwards an update to all entities managed under the root
		 */
		virtual void update();

	private:
		using InstanceByIDMap	= std::unordered_map<std::string, rtti::RTTIObject*>;					// Map from object ID to object (non-owned)
		using ObjectByIDMap		= std::unordered_map<std::string, std::unique_ptr<rtti::RTTIObject>>;	// Map from object ID to object (owned)
		using FileLinkMap		= std::unordered_map<std::string, std::vector<std::string>>;			// Map from target file to multiple source files

		enum class EFileModified : uint8_t
		{
			Yes,
			No,
			Error
		};

		void addObject(const std::string& id, std::unique_ptr<rtti::RTTIObject> object);
		void removeObject(const std::string& id);
		void addFileLink(const std::string& sourceFile, const std::string& targetFile);

		void determineObjectsToInit(const RTTIObjectGraph& objectGraph, const ObjectByIDMap& objectsToUpdate, const std::string& externalChangedFile, std::vector<std::string>& objectsToInit);
		bool resolvePointers(ObjectByIDMap& objectsToUpdate, const rtti::UnresolvedPointerList& unresolvedPointers, utility::ErrorState& errorState);
		bool initObjects(const std::vector<std::string>& objectsToInit, const ObjectByIDMap& objectsToUpdate, utility::ErrorState& errorState);
		bool initEntities(const RTTIObjectGraph& objectGraph, const ObjectByIDMap& objectsToUpdate, utility::ErrorState& errorState);
		bool createEntities(const std::vector<const Entity*>& entityResources, EntityCreationParameters& entityCreationParams, std::vector<std::string>& generatedEntityIDs, utility::ErrorState& errorState);
		static bool sResolveComponentPointers(EntityCreationParameters& entityCreationParams, std::unordered_map<Component*, ComponentInstance*>& newComponentInstances, utility::ErrorState& errorState);
		bool buildObjectGraph(const ObjectByIDMap& objectsToUpdate, RTTIObjectGraph& objectGraph, utility::ErrorState& errorState);
		EFileModified isFileModified(const std::string& modifiedFile);

		/** 
		* Traverses all pointers in ObjectPtrManager and, for each target, replaces the target with the one in the map that is passed.
		* @param container The container holding an ID -> pointer mapping with the pointer to patch to.
		*/
		template<class OBJECTSBYIDMAP> 
		void patchObjectPtrs(OBJECTSBYIDMAP& newTargetObjects);

	private:

		/**
		 * Helper class that patches object pointers back to the objects as present in the resource manager.
		 * When clear is called, no rollback is performed.
		 */
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

		using ModifiedTimeMap = std::unordered_map<std::string, uint64>;

		std::unique_ptr<EntityInstance>		mRootEntity;					// Root entity, owned and created by the system
		ObjectByIDMap						mObjects;						// Holds all objects
		EntityByIDMap						mEntities;						// Holds all entitites
		std::set<std::string>				mFilesToWatch;					// Files currently loaded, used for watching changes on the files
		FileLinkMap							mFileLinkMap;					// Map containing links from target to source file, for updating source files if the file monitor sees changes
		std::unique_ptr<DirectoryWatcher>	mDirectoryWatcher;				// File monitor, detects changes on files
		double								mLastTimeStamp = 0;				// Last time stamp used for calculating delta time
		ModifiedTimeMap						mFileModTimes;					// Cache for file modification times to avoid responding to too many file events
		std::unique_ptr<rtti::Factory>		mFactory;						// Responsible for creating objects when de-serializing
		Core&								mCore;							// Core
	};


	template<class OBJECTSBYIDMAP>
	void ResourceManagerService::patchObjectPtrs(OBJECTSBYIDMAP& newTargetObjects)
	{
		ObjectPtrManager::ObjectPtrSet& object_ptrs = ObjectPtrManager::get().GetObjectPointers();

		for (ObjectPtrBase* ptr : object_ptrs)
		{
			rtti::RTTIObject* target = ptr->get();
			if (target == nullptr)
				continue;

			std::string& target_id = target->mID;
			typename OBJECTSBYIDMAP::iterator new_target = newTargetObjects.find(target_id);
			if (new_target != newTargetObjects.end())
				ptr->set(&*(new_target->second));
		}
	}
}
