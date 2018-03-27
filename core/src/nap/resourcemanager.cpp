#include "resourcemanager.h"
#include "objectgraph.h"
#include "logger.h"
#include "core.h"
#include "rttiobjectgraphitem.h"
#include <utility/fileutils.h>
#include <utility/stringutils.h>
#include <rtti/rttiutilities.h>
#include <rtti/jsonreader.h>
#include <rtti/pythonmodule.h>
#include <rtti/linkresolver.h>
#include <nap/core.h>
#include <nap/device.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ResourceManager)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_FUNCTION("findObject", (const nap::rtti::ObjectPtr<nap::rtti::Object> (nap::ResourceManager::*)(const std::string&))&nap::ResourceManager::findObject)
RTTI_END_CLASS

namespace nap
{
	using namespace rtti;

	//////////////////////////////////////////////////////////////////////////
	// ResourceManager::LinkResolver
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Implementation of LinkResolver that resolves UnresolvedPointers against the combination of the ResourceManager's objects and the ObjectsToUpdate map.
	 * The ObjectsToUpdate map functions as an overlay on top of the ResourceManager.
	 */
	class ResourceManager::OverlayLinkResolver : public LinkResolver
	{
	public:
		OverlayLinkResolver(ResourceManager& resourceManager, ObjectByIDMap& objectsToUpdate) :
			mResourceManager(&resourceManager),
			mObjectsToUpdate(&objectsToUpdate)
		{
		}

	private:
		virtual Object* findTarget(const std::string& targetID) override
		{
			// Objects in objectsToUpdate have preference over the manager's objects. 
			auto object_to_update = mObjectsToUpdate->find(targetID);
			if (object_to_update == mObjectsToUpdate->end())
				return mResourceManager->findObject(targetID).get();

			return object_to_update->second.get();
		}

	private:
		ResourceManager*	mResourceManager;
		ObjectByIDMap*		mObjectsToUpdate;
	};

	//////////////////////////////////////////////////////////////////////////
	// ResourceManager::RollbackHelper
	//////////////////////////////////////////////////////////////////////////


	ResourceManager::RollbackHelper::RollbackHelper(ResourceManager& service) :
		mService(service)
	{
	}


	ResourceManager::RollbackHelper::~RollbackHelper()
	{
		if (!mRollbackObjects)
			return;

		for (Device* new_device : mNewDevices)
			new_device->stop();

		utility::ErrorState errorState;
		for (Device* existing_device : mExistingDevices)
		{
			bool result = existing_device->start(errorState);
			assert(result);
		}

		rtti::ObjectPtrManager::get().patchPointers(mService.mObjects);
	}


	void ResourceManager::RollbackHelper::clear()
	{
		mRollbackObjects = false;
	}


	void ResourceManager::RollbackHelper::addExistingDevice(Device& device)
	{
		mExistingDevices.push_back(&device);
	}

	void ResourceManager::RollbackHelper::addNewDevice(Device& device)
	{
		mNewDevices.push_back(&device);
	}

	//////////////////////////////////////////////////////////////////////////


	/**
	 * Performs a graph traversal of all objects in the graph that have the ID that matches any of the objects in @param dirtyObjects.
     * All the edges in the graph are traversed in the incoming direction. Any object that is encountered is added to the set.
     * Finally, all objects that were visited are sorted on graph depth.
 	 */
	static void sTraverseAndSortIncomingObjects(const std::unordered_map<std::string, rtti::Object*>& dirtyObjects, const RTTIObjectGraph& objectGraph, std::vector<std::string>& sortedObjects)
	{
		// Traverse graph for incoming links and add all of them
		std::set<RTTIObjectGraph::Node*> nodes;
		for (auto& kvp : dirtyObjects)
		{
			RTTIObjectGraph::Node* node = objectGraph.findNode(kvp.first);

			// In the case that file links change as part of the file modification(s), it's possible for the dirty node to not be present in the ObjectGraph,
			// so we can't assert here but need to deal with that case.
			if (node != nullptr)
				RTTIObjectGraph::addIncomingObjectsRecursive(node, nodes);
		}

		// Sort on graph depth for the correct init() order
		std::vector<RTTIObjectGraph::Node*> sorted_nodes;
		for (RTTIObjectGraph::Node* object_to_init : nodes)
			sorted_nodes.push_back(object_to_init);

		std::sort(sorted_nodes.begin(), sorted_nodes.end(),
			[](RTTIObjectGraph::Node* nodeA, RTTIObjectGraph::Node* nodeB) { return nodeA->mDepth > nodeB->mDepth; });

		for (RTTIObjectGraph::Node* sorted_object_to_init : sorted_nodes)
			if (sorted_object_to_init->mItem.mObject != nullptr)
				sortedObjects.push_back(sorted_object_to_init->mItem.mObject->mID);
	}


	//////////////////////////////////////////////////////////////////////////
	// ResourceManager
	//////////////////////////////////////////////////////////////////////////


	ResourceManager::ResourceManager(nap::Core& core) :
		mDirectoryWatcher(std::make_unique<DirectoryWatcher>()),
		mFactory(std::make_unique<Factory>()),
		mCore(core)
	{
	}


	/**
	 * Add all objects from the resource manager into an object graph, overlayed by @param objectsToUpdate.
 	 */
	bool ResourceManager::buildObjectGraph(const ObjectByIDMap& objectsToUpdate, RTTIObjectGraph& objectGraph, utility::ErrorState& errorState)
	{
		// Build an object graph of all objects in the ResourceMgr. If any object is in the objectsToUpdate list,
		// that object is added instead. This makes objectsToUpdate and 'overlay'.
		ObservedObjectList all_objects;
		for (auto& kvp : mObjects)
		{
			ObjectByIDMap::const_iterator object_to_update = objectsToUpdate.find(kvp.first);
			if (object_to_update == objectsToUpdate.end())
				all_objects.push_back(kvp.second.get());
			else
				all_objects.push_back(object_to_update->second.get());
		}

		// Any objects not yet in the manager are new and need to be added to the graph as well
		for (auto& kvp : objectsToUpdate)
			if (mObjects.find(kvp.first) == mObjects.end())
				all_objects.push_back(kvp.second.get());

		if (!objectGraph.build(all_objects, [](rtti::Object* object) { return RTTIObjectGraphItem::create(object); }, errorState))
			return false;

		return true;
	}
	

	/**
	 * From all objects that are effectively changed or added, traverses the object graph to find the minimum set of objects that requires an init. 
	 * The list of objects is sorted on object graph depth so that the init() order is correct.
	 */
	void ResourceManager::determineObjectsToInit(const RTTIObjectGraph& objectGraph, const ObjectByIDMap& objectsToUpdate, const std::string& externalChangedFile, std::vector<std::string>& objectsToInit)
	{
		// Mark all the objects to update as 'dirty', we need to init() those and 
		// all the objects that point to them (recursively)
		std::unordered_map<std::string, rtti::Object*> dirty_nodes;
		for (auto& kvp : objectsToUpdate)
			dirty_nodes.insert(std::make_pair(kvp.first, kvp.second.get()));

		// Add externally changed file that caused load of this json file
		if (!externalChangedFile.empty())
			dirty_nodes.insert(std::make_pair(externalChangedFile, nullptr));

		// Traverse graph for incoming links and add all of them, and sort them based on graph depth
		sTraverseAndSortIncomingObjects(dirty_nodes, objectGraph, objectsToInit);
	}


	bool ResourceManager::loadFile(const std::string& filename, utility::ErrorState& errorState)
	{
		return loadFile(filename, std::string(), errorState);
	}


	bool ResourceManager::loadFile(const std::string& filename, const std::string& externalChangedFile, utility::ErrorState& errorState)
	{
		// ExternalChangedFile should only be used if it's different from the file being reloaded
		assert(utility::toComparableFilename(filename) != utility::toComparableFilename(externalChangedFile));

		// Read objects from disk
		DeserializeResult read_result;
		if (!readJSONFile(filename, getFactory(), read_result, errorState))
			return false;

		// We first gather the objects that require an update. These are the new objects and the changed objects.
		// Change detection is performed by comparing RTTI attributes. Very important to note is that, after reading
		// a json file, pointers are unresolved. When comparing them to the existing objects, they are always different
		// because in the existing objects the pointers are already resolved.
		// To solve this, we have a special RTTI comparison function that receives the unresolved pointer list from readJSONFile.
		// Internally, when a pointer is found, the ID of the unresolved pointer is checked against the mID of the target object.
		//
		// The reason why we cannot first resolve and then compare, is because deciding what to resolve against what objects
		// depends on the dirty comparison.
		// Finally, we could improve on the unresolved pointer check if we could introduce actual UnresolvedPointer objects
		// that the pointers are pointing to after loading. These would hold the ID, so that comparisons could be made easier.
		// The reason we don't do this is because it isn't possible to do so in RTTR as it's very strict in it's type safety.
		ObjectByIDMap objects_to_update;
		for (auto& read_object : read_result.mReadObjects)
		{
			std::string id = read_object->mID;

			ObjectByIDMap::iterator existing_object = mObjects.find(id);
			if (existing_object == mObjects.end() ||
				!areObjectsEqual(*read_object.get(), *existing_object->second.get(), read_result.mUnresolvedPointers))
			{
				objects_to_update.emplace(std::make_pair(id, std::move(read_object)));
			}
		}


		// Resolve all unresolved pointers. The set of objects to resolve against are the objects in the ResourceManager, with the new/dirty
		// objects acting as an overlay on the existing objects. In other words, when resolving, objects read from the json file have 
		// preference over objects in the resource manager as these are the ones that will eventually be (re)placed in the manager.
		OverlayLinkResolver resolver(*this, objects_to_update);
		if (!resolver.resolveLinks(read_result.mUnresolvedPointers, errorState))
			return false;

		// We instantiate a helper that will perform a rollback of any pointer patching that we have done.
		// In case of success we clear this helper.
		// We only ever need to rollback the pointer patching, because the resource manager remains untouched
		// until the very end where we know that all init() calls have succeeded
		RollbackHelper rollback_helper(*this);

		// Patch ObjectPtrs so that they point to the updated object instead of the old object. We need to do this before determining
		// init order, otherwise a part of the graph may still be pointing to the old objects.
		rtti::ObjectPtrManager::get().patchPointers(objects_to_update);

		// Build object graph of all the objects in the manager, overlayed by the objects we want to update. Later, we will
		// performs queries against this graph to determine init order for both resources and entities.
		RTTIObjectGraph object_graph;
		if (!buildObjectGraph(objects_to_update, object_graph, errorState))
			return false;

		// Find out what objects to init and in what order to init them
		std::vector<std::string> objects_to_init;
		determineObjectsToInit(object_graph, objects_to_update, externalChangedFile, objects_to_init);

		// The objects that require an init may contain objects that were not present in the file (because they are
		// pointing to objects that will be reconstructed and initted). In that case we reconstruct those objects 
		// as well by cloning them and pushing them into the objects_to_update list.
		for (const std::string& object_to_init : objects_to_init)
		{
			if (objects_to_update.find(object_to_init) == objects_to_update.end())
			{
				Object* object = findObject(object_to_init).get();
				assert(object);

				std::unique_ptr<Object> cloned_object = rtti::cloneObject(*object, getFactory());
				
				// Replace original object in object graph with the cloned version. This fixes problems when real-time editing components. 
				RTTIObjectGraph::Node* originalObjectNode = object_graph.findNode(object->mID);
				assert(originalObjectNode && originalObjectNode->mItem.mType == RTTIObjectGraphItem::EType::Object);

				originalObjectNode->mItem.mObject = cloned_object.get();
				objects_to_update.emplace(std::make_pair(cloned_object->mID, std::move(cloned_object)));
			}
		}

		// Stop all existing devices that are about to be updated
		for (auto& kvp : objects_to_update)
		{
			if (!kvp.second->get_type().is_derived_from<Device>())
				continue;

			ObjectByIDMap::iterator existing_object = mObjects.find(kvp.first);
			if (existing_object != mObjects.end())
			{
				Device* device = (Device*)(existing_object->second.get());
				device->stop();

				// Devices that are stopped need to be (re)started when an error occurs
				rollback_helper.addExistingDevice(*device);
			}
		}

		// Patch again to update pointers to objects that were cloned
		rtti::ObjectPtrManager::get().patchPointers(objects_to_update);

		// Init all objects in the correct order and start devices at the same time
		for (const std::string& id : objects_to_init)
		{
			rtti::Object* object = nullptr;

			ObjectByIDMap::const_iterator pos = objects_to_update.find(id);
			assert(pos != objects_to_update.end());
			
			object = pos->second.get();
			if (!errorState.check(object->init(errorState), "Couldn't initialize object '%s'", id.c_str()))
				return false;

			if (object->get_type().is_derived_from<Device>())				
			{
				Device* device = (Device*)object;
				if (!errorState.check(device->start(errorState), "Couldn't start device '%s'", id.c_str()))
					return false;

				// Any device that is successfully started needs to be stopped when an error occurs
				rollback_helper.addNewDevice(*device);
			}
		}

		// In case all init() operations were successful, we can now replace the objects
		// in the manager by the new objects. This effectively destroys the old objects as well.
		for (auto& kvp : objects_to_update)
			mObjects[kvp.first] = std::move(kvp.second);

		for (const FileLink& file_link : read_result.mFileLinks)
			addFileLink(filename, file_link.mTargetFile);

		mFilesToWatch.insert(utility::toComparableFilename(filename));
		
		// Everything was successful, don't rollback any changes that were made
		rollback_helper.clear();

		// Notify listeners
		mFileLoadedSignal.trigger(filename);

		return true;
	}

	ResourceManager::EFileModified ResourceManager::isFileModified(const std::string& modifiedFile)
	{
		// Get file time
		uint64 mod_time;
		bool can_get_mod_time = utility::getFileModificationTime(modifiedFile, mod_time);
		if (!can_get_mod_time)
			return EFileModified::Error;
		
		std::string comparableFilename = utility::toComparableFilename(modifiedFile);

		// Check if filetime is in the cache
		ModifiedTimeMap::iterator pos = mFileModTimes.find(comparableFilename);
		if (pos == mFileModTimes.end())
		{
			// No, file must be dirty. Insert into cache
			mFileModTimes.insert(std::make_pair(comparableFilename, mod_time));
			return EFileModified::Yes;
		}
		else
		{
			// File is in the cache, but has it changed since the last call to isFileModified?
			if (pos->second != mod_time)
			{
				pos->second = mod_time;
				return EFileModified::Yes;
			}
		}
		return EFileModified::No;
	}

	void ResourceManager::checkForFileChanges()
	{
		std::vector<std::string> modified_files;
		if (mDirectoryWatcher->update(modified_files))
		{
			for (std::string& modified_file : modified_files)
			{
				// Multiple events for the same file may occur, and we do not want to reload for every event given.
				// Instead we check the filetime and store that filetime in an internal map. If an event comes by that
				// with a filetime that we already processed, we ignore it.
				// It may also be possible that events are thrown for files that we do not have access to, or for files
				// that have been removed in the meantime. For these cases, we ignore events where the filetime check
				// fails.
				EFileModified file_modified = isFileModified(modified_file);
				if (file_modified == EFileModified::Error || file_modified == EFileModified::No)
					continue;

				modified_file = utility::toComparableFilename(modified_file);
				std::set<std::string> files_to_reload;

				// Is our modified file a json file that was loaded by the manager?
				if (mFilesToWatch.find(modified_file) != mFilesToWatch.end())
				{
					files_to_reload.insert(modified_file);
				}
				else
				{
					// Non-json file. Find all the json sources of this file
					FileLinkMap::iterator file_link = mFileLinkMap.find(modified_file);
					if (file_link != mFileLinkMap.end())
						for (const std::string& source_file : file_link->second)
							files_to_reload.insert(source_file);
				}

				if (!files_to_reload.empty())
				{
					nap::Logger::info("Detected change to %s. Files needing reload:", modified_file.c_str());
					for (const std::string& source_file : files_to_reload)
						nap::Logger::info("\t-> %s", source_file.c_str());

					for (const std::string& source_file : files_to_reload)
					{
						utility::ErrorState errorState;
						if (!loadFile(source_file, source_file == modified_file ? std::string() : modified_file, errorState))
						{
							nap::Logger::warn("Failed to reload %s (%s)", source_file.c_str(), errorState.toString().c_str());
							break;
						}
					}
				}
			}
		}
	}


	nap::rtti::Factory& ResourceManager::getFactory()
	{
		return *mFactory;
	}


	const rtti::ObjectPtr<Object> ResourceManager::findObject(const std::string& id)
	{
		const auto& it = mObjects.find(id);
		
		if (it != mObjects.end())
			return rtti::ObjectPtr<Object>(it->second.get());
		
		return nullptr;
	}


	void ResourceManager::addObject(const std::string& id, std::unique_ptr<Object> object)
	{
		assert(mObjects.find(id) == mObjects.end());
		mObjects.emplace(id, std::move(object));
	}


	void ResourceManager::removeObject(const std::string& id)
	{
		assert(mObjects.find(id) != mObjects.end());
		mObjects.erase(mObjects.find(id));
	}


	void ResourceManager::addFileLink(const std::string& sourceFile, const std::string& targetFile)
	{
		std::string source_file = utility::toComparableFilename(sourceFile);
		std::string target_file = utility::toComparableFilename(targetFile);
		
		FileLinkMap::iterator existing = mFileLinkMap.find(targetFile);
		if (existing == mFileLinkMap.end())
		{
			std::vector<std::string> source_files;
			source_files.push_back(source_file);

			mFileLinkMap.insert({ target_file, source_files });
		}
		else
		{
			existing->second.push_back(source_file);
		}
	}


	const rtti::ObjectPtr<Object> ResourceManager::createObject(const rtti::TypeInfo& type)
	{
		if (!type.is_derived_from(RTTI_OF(Object)))
		{
			nap::Logger::warn("unable to create object of type: %s", type.get_name().data());
			return nullptr;
		}

		if (!getFactory().canCreate(type))
		{
			nap::Logger::warn("can't create object instance of type: %s", type.get_name().data());
			return nullptr;
		}

		// Create instance of object
		Object* object = rtti_cast<Object>(getFactory().create(type));

		// Construct path
		std::string type_name = type.get_name().data();
		std::string reso_path = utility::stringFormat("object::%s", type_name.c_str());
		std::string reso_unique_path = reso_path;
		int idx = 0;
		while (mObjects.find(reso_unique_path) != mObjects.end())
		{
			++idx;
			reso_unique_path = utility::stringFormat("%s_%d", reso_path.c_str(), idx);
		}

		object->mID = reso_unique_path;
		addObject(reso_unique_path, std::unique_ptr<Object>(object));
		
		return rtti::ObjectPtr<Object>(object);
	}

}
