/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "numeric.h"
#include "signalslot.h"
#include "corefactory.h"
#include "directorywatcher.h"

// External Includes
#include <rtti/rtti.h>
#include <rtti/objectptr.h>
#include <utility/dllexport.h>
#include <rtti/unresolvedpointer.h>
#include <rtti/factory.h>
#include <rtti/deserializeresult.h>
#include <map>

namespace nap
{	
	class Core;
	class Scene;
	class Device;

	class RTTIObjectGraphItem;
	template<typename ITEM> class ObjectGraph;
	using RTTIObjectGraph = ObjectGraph<RTTIObjectGraphItem>;

	/**
	 * The resource manager is responsible for loading a JSON file that contains all the resources that are necessary for an application to run. 
	 *
	 * When loading a JSON file all the objects declared inside that file are created and initialized by the resource manager. 
	 * These objects are called 'resources'. Every loaded resource is owned by the resource manager. 
	 * This means that the lifetime of a resource is fully managed by the resource manager and not by the client.
	 * The resource manager also updates the content in real-time when a change to the loaded JSON file is detected.
	 * 
	 * Every resource has a unique identifier, as declared by the 'mID' property. The name of the object is required to be unique.
	 * De-serialization will fail when a duplicate object ID is discovered. 
	 *
	 * The most important task of a resource is to tell the resource manager if initialization succeeded. 
	 * If initialization of a resource fails the resource manager will halt execution, return an error message and 
	 * as a result stop further execution of a program.
	 *
	 * File load example:
	 * 
	 *~~~~~{.cpp}
	 *	// Get resource manager and load application JSON
	 *	mResourceManager = getCore().getResourceManager();
	 *	if (!mResourceManager->loadFile("helloworld.json", error))
	 *		return false;
	 *~~~~~
	 */
	class NAPAPI ResourceManager final
	{
		friend class Core;
		RTTI_ENABLE()
	public:
		ResourceManager(nap::Core& core);

		~ResourceManager();

		/**
		 * Helper that calls loadFile without additional modified objects. See loadFile() comments for a full description.
		 * @param fileName JSON resource file to load.
		 * @param errorState contains the error when the load operation fails.
		 * @return if the file loaded successfully.
		 */
		bool loadFile(const std::string& filename, utility::ErrorState& errorState);

		/**
		 * Helper that calls loadFile without additional modified objects. See loadFile() comments for a full description.
		 * @param fileName JSON resource file to load.
		 * @return if the file loaded successfully.
		 */
        bool loadFile(const std::string& filename);

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
		* Before objects are destructed, onDestroy is called. onDestroy is called in the reverse initialization order. This way, it is still safe to use any 
		* pointers to perform cleanup of internal data. 
		*
		* @param filename json file containing all objects.
		* @param externalChangedFile externally changed file that caused load of this file (like texture, shader etc)
		* @param errorState if the function returns false, contains error information.
		* @return if the file loaded successfully.
		*/
		bool loadFile(const std::string& filename, const std::string& externalChangedFile, utility::ErrorState& errorState);

		/**
		 * Find an object by object ID. Returns null if not found.
		 * @param id unique id of the object to find.
		 * @return the object, nullptr if not found.
		 */
		const rtti::ObjectPtr<rtti::Object> findObject(const std::string& id);

		/**
		 * Find an object of type T. Returns null if not found.
		 *
		 * Example:
		 *
		 *~~~~~{.cpp}
		 * mResourceManager = getCore().getResourceManager();
		 * auto window = mResourceManager->findObject<nap::RenderWindow>("Window0");
		 *~~~~~
		 *
		 * @param id the unique id of the object to find.
		 * @return the object of type T, nullptr if not found.
		 */
		template<class T>
		const rtti::ObjectPtr<T> findObject(const std::string& id) { return rtti::ObjectPtr<T>(findObject(id)); }

		/**
		 * Get all objects of a particular type.
		 * @return all objects of the requested type.
		 */
		template<class T>
		std::vector<rtti::ObjectPtr<T>> getObjects() const;

		/**
		 * Creates an object and adds it to the manager.
		 * @param type object type to create.
		 * @return newly created object.
		 */
		const rtti::ObjectPtr<rtti::Object> createObject(const rtti::TypeInfo& type);

		/**
		 * Creates an object of type T and adds it to the manager.
		 * @return the newly created object.
		 */
		template<typename T>
		const rtti::ObjectPtr<T> createObject() { return rtti::ObjectPtr<T>(createObject(RTTI_OF(T))); }

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
		 * Select the current working directory to monitor for file changes.
		 * All files linked to by the application that reside in this directory will be monitored.
		 * If a file change is detected to any of the files, the resource manager
		 * will attempt to hot-load the changes directly into the running application
		 * @param directory the directory to monitor
		 * @param error contains the error if the operation fails
		 * @return if the directory is being monitored
		 */
		void watchDirectory();

	private:
		using InstanceByIDMap	= std::unordered_map<std::string, rtti::Object*>;					// Map from object ID to object (non-owned)
		using ObjectByIDMap		= std::unordered_map<std::string, std::unique_ptr<rtti::Object>>;	// Map from object ID to object (owned)
		using FileLinkMap		= std::unordered_map<std::string, std::vector<std::string>>;		// Map from target file to multiple source files

		class OverlayLinkResolver;

		enum class EFileModified : uint8_t
		{
			Yes,
			No,
			Error
		};

		void addObject(const std::string& id, std::unique_ptr<rtti::Object> object);
		void removeObject(const std::string& id);
		void addFileLink(const std::string& sourceFile, const std::string& targetFile);

		/**
		* Lower level platform dependent function used by loadFile that simply loads the file from disk and deserializes.
		*/
		bool loadFileAndDeserialize(const std::string& filename, rtti::DeserializeResult& readResult, utility::ErrorState& errorState);

		void determineObjectsToInit(const RTTIObjectGraph& objectGraph, const ObjectByIDMap& objectsToUpdate, const std::string& externalChangedFile, std::vector<std::string>& objectsToInit);

		void buildObjectGraph(const ObjectByIDMap& objectsToUpdate, RTTIObjectGraph& objectGraph);
		EFileModified isFileModified(const std::string& modifiedFile);

		void stopAndDestroyAllObjects();
		void destroyObjects(const std::unordered_set<std::string>& objectIDsToDelete, const RTTIObjectGraph& object_graph);

	private:

		/**
		 * Helper class that patches object pointers back to the objects as present in the resource manager.
		 * When clear is called, no rollback is performed.
		 */
		struct RollbackHelper final
		{
		public:
			RollbackHelper(ResourceManager& service);
			~RollbackHelper();

			void clear();
			void addExistingDevice(Device& device);
			void addNewDevice(Device& device);
			void addInitializedObject(rtti::Object& object);

			ObjectByIDMap& getObjectsToUpdate() { return mObjectsToUpdate; }

		private:
			void destroyObjects();

		private:
			ResourceManager&			mService;
			ObjectByIDMap				mObjectsToUpdate;			///< Owned map of all objects that need to be pushed into the ResourceManager.
			std::vector<Device*>		mExistingDevices;			///< This is the list of devices that *already exist* in the ResourceManager which will be updated
			std::vector<Device*>		mNewDevices;				///< This is the list of devices that have been newly read from the json file, which contain the updated versions of the existing devices
			std::vector<rtti::Object*>	mInitializedObjects;		///< This is the list of objects that have actually been initted and will need onDestroy called on them in case of an error
			bool						mRollbackObjects = true;
		};

		using ModifiedTimeMap = std::unordered_map<std::string, uint64>;

		ObjectByIDMap						mObjects;						// Holds all objects
		std::set<std::string>				mFilesToWatch;					// Files currently loaded, used for watching changes on the files
		FileLinkMap							mFileLinkMap;					// Map containing links from target to source file, for updating source files if the file monitor sees changes
		std::unique_ptr<DirectoryWatcher>	mDirectoryWatcher = nullptr;	// File monitor, detects changes on files
		ModifiedTimeMap						mFileModTimes;					// Cache for file modification times to avoid responding to too many file events
		std::unique_ptr<CoreFactory>		mFactory = nullptr;				// Responsible for creating objects when de-serializing
		Core&								mCore;							// Core

		/**
		 *	Signal that is emitted when a file is about to be loaded
		 */
		nap::Signal<> mPreResourcesLoadedSignal;

		/**
		 *	Signal that is emitted after a file has been successfully loaded
		 */
		nap::Signal<> mPostResourcesLoadedSignal;
	};

	template<class T>
	std::vector<rtti::ObjectPtr<T>> ResourceManager::getObjects() const
	{
		std::vector<rtti::ObjectPtr<T>> result;
		for (auto& kvp : mObjects)
		{
			T* object = rtti_cast<T>(kvp.second.get());
			if (object != nullptr)
				result.push_back(object);
		}

		return result;
	}
}
