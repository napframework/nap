#pragma once

#include "fileutils.h"
#include "logger.h"
#include "object.h"
#include "service.h"
#include "resource.h"
#include "rtti/jsonreader.h"
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
		* Loads a json file containing objects. All objects are added to the manager. If objects already exist (which is checked by their ID), than the
		* objects are updated. If objects already exist but are unchanged, they are not updated.
		*
		* After objects are created or updated, init() is called for all these objects. init() is called in the correct dependency order: if an object
		* has a pointer to another object, the pointee is initted first. Note that it may be required that init() is called for objects that do not exist
		* in the file, but were already existing in the manager: if object A points to object B, and object B is loaded from file, then object A must be
		* initted as well, as it may rely on new data from object B.
		* 
		* If there were no errors during the loading and updating process, finish(COMMIT) is called on all objects.
		* If errors did occur, finish(ROLLBACK) is called, and the objects are responsible for returning their internal state back to the state before init()
		* was called. NOTE though, that loadFile will rollback any RTTI attribute values for you already, so this is only about rolling back internal non-rtti state.
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
		friend class ObjectRestorer;

		using ClonedObjectMap = std::map<RTTIObject*, std::unique_ptr<RTTIObject>>;
		using ExistingObjectMap = std::map<RTTIObject*, RTTIObject*>;
		
		void splitFileObjects(rtti::OwnedObjectList& fileObjects, ExistingObjectMap& existingObjects, rtti::ObservedObjectList& newObjects);
		bool determineObjectsToInit(const ExistingObjectMap& existingObjects, const ClonedObjectMap& clonedObjects, const rtti::ObservedObjectList& newObjects, const std::string& externalChangedFile, rtti::ObservedObjectList& objectsToInit, utility::ErrorState& errorState);
		bool updateExistingObjects(const ExistingObjectMap& existingObjectMap, rtti::UnresolvedPointerList& unresolvedPointers, utility::ErrorState& errorState);
		void addResource(const std::string& id, std::unique_ptr<Resource> resource);
		void removeResource(const std::string& id);
		void addFileLink(const std::string& sourceFile, const std::string& targetFile);

	private:
		using ResourceMap = std::map<std::string, std::unique_ptr<Resource>>;
		using FileLinkMap = std::map<std::string, std::vector<std::string>>; // Map from target file to multiple source files

		ResourceMap							mResources;				// Holds all resources
		std::set<std::string>				mFilesToWatch;			// Files currently loaded, used for watching changes on the files
		FileLinkMap							mFileLinkMap;			// Map containing links from target to source file, for updating source files if the file monitor sees changes
		std::unique_ptr<DirectoryWatcher>	mDirectoryWatcher;		// File monitor, detects changes on files
	};

}
