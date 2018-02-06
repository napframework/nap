#pragma once

// Local Includes
#include "rtti/rtti.h"
#include "objectptr.h"
#include "utility/dllexport.h"
#include "directorywatcher.h"
#include "configure.h"
#include "signalslot.h"

// External Includes
#include <rtti/unresolvedpointer.h>
#include <rtti/factory.h>
#include <map>

namespace nap
{	
	class Core;
	class Scene;

	class RTTIObjectGraphItem;
	template<typename ITEM> class ObjectGraph;
	using RTTIObjectGraph = ObjectGraph<RTTIObjectGraphItem>;

	/**
	 * Manager, owner of all objects, capable of loading and real-time updating of content.
	 */
	class NAPAPI ResourceManager
	{
		friend class Core;
		RTTI_ENABLE()
	public:
		ResourceManager(nap::Core& core);

		/**
		* Helper that calls loadFile without additional modified objects. See loadFile comments for a full description.
		*/
		bool loadFile(const std::string& filename, utility::ErrorState& errorState);

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
		using InstanceByIDMap	= std::unordered_map<std::string, rtti::RTTIObject*>;					// Map from object ID to object (non-owned)
		using ObjectByIDMap		= std::unordered_map<std::string, std::unique_ptr<rtti::RTTIObject>>;	// Map from object ID to object (owned)
		using FileLinkMap		= std::unordered_map<std::string, std::vector<std::string>>;			// Map from target file to multiple source files

		class OverlayLinkResolver;

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
		bool initObjects(const std::vector<std::string>& objectsToInit, const ObjectByIDMap& objectsToUpdate, utility::ErrorState& errorState);

		bool buildObjectGraph(const ObjectByIDMap& objectsToUpdate, RTTIObjectGraph& objectGraph, utility::ErrorState& errorState);
		EFileModified isFileModified(const std::string& modifiedFile);

		
	private:

		/**
		 * Helper class that patches object pointers back to the objects as present in the resource manager.
		 * When clear is called, no rollback is performed.
		 */
		struct RollbackHelper
		{
		public:
			RollbackHelper(ResourceManager& service);
			~RollbackHelper();

			void clear();

		private:
			ResourceManager& mService;
			bool mPatchObjects = true;
		};

		using ModifiedTimeMap = std::unordered_map<std::string, uint64>;

		ObjectByIDMap						mObjects;						// Holds all objects
		std::set<std::string>				mFilesToWatch;					// Files currently loaded, used for watching changes on the files
		FileLinkMap							mFileLinkMap;					// Map containing links from target to source file, for updating source files if the file monitor sees changes
		std::unique_ptr<DirectoryWatcher>	mDirectoryWatcher;				// File monitor, detects changes on files
		ModifiedTimeMap						mFileModTimes;					// Cache for file modification times to avoid responding to too many file events
		std::unique_ptr<rtti::Factory>		mFactory;						// Responsible for creating objects when de-serializing
		Core&								mCore;							// Core

		/**
		 *	Signal that is emitted when a file has been successfully loaded
		 */
		nap::Signal<const std::string&> mFileLoadedSignal;
	};
}
