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

	/**
	 * Manager, holding all objects, capable of loading and real-time updating of content.
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

	private:
		using ObjectByIDMap = std::map<std::string, std::unique_ptr<RTTIObject>>;
		using FileLinkMap = std::map<std::string, std::vector<std::string>>; // Map from target file to multiple source files

		void addObject(const std::string& id, std::unique_ptr<RTTIObject> object);
		void removeObject(const std::string& id);
		void addFileLink(const std::string& sourceFile, const std::string& targetFile);

		bool determineObjectsToInit(const ObjectByIDMap& objectsToUpdate, const std::string& externalChangedFile, std::vector<std::string>& objectsToInit, utility::ErrorState& errorState);
		bool resolvePointers(ObjectByIDMap& objectsToUpdate, const rtti::UnresolvedPointerList& unresolvedPointers, utility::ErrorState& errorState);
		bool initObjects(std::vector<std::string> objectsToInit, ObjectByIDMap& objectsToUpdate, utility::ErrorState& errorState);
		void patchObjectPtrs(ObjectByIDMap& newTargetObjects);

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

		ObjectByIDMap						mObjects;				// Holds all objects
		std::set<std::string>				mFilesToWatch;			// Files currently loaded, used for watching changes on the files
		FileLinkMap							mFileLinkMap;			// Map containing links from target to source file, for updating source files if the file monitor sees changes
		std::unique_ptr<DirectoryWatcher>	mDirectoryWatcher;		// File monitor, detects changes on files
	};
}
