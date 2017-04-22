#include "resourcemanager.h"
#include "rttiutilities.h"
#include "directorywatcher.h"
#include "jsonreader.h"


RTTI_DEFINE(nap::ResourceManagerService)

namespace nap
{

	ResourceManagerService::ResourceManagerService() :
		mDirectoryWatcher(new DirectoryWatcher())
	{
	}

	void ResourceManagerService::splitObjects(const ObjectList& sourceObjectList, ObjectList& targetObjectList, ExistingObjectMap& existingObjectMap, ObjectList& newObjectList)
	{
		for (Object* source_object : sourceObjectList)
		{
			Resource* resource = rtti_cast<Resource>(source_object);
			if (resource == nullptr)
				continue;

			std::string id = resource->mID;

			Resource* existing_resource = findResource(id);
			if (existing_resource == nullptr)
			{
				newObjectList.push_back(source_object);
				targetObjectList.push_back(source_object);
			}
			else
			{
				existingObjectMap.insert({ source_object, existing_resource });
				targetObjectList.push_back(existing_resource);
			}
		}
	}

	static int findUnresolvedPointer(UnresolvedPointerList& unresolvedPointers, Object* object, const RTTI::Property& property)
	{
		for (int index = 0; index < unresolvedPointers.size(); ++index)
		{
			UnresolvedPointer& unresolved_pointer = unresolvedPointers[index];
			if (unresolved_pointer.mObject == object &&
				unresolved_pointer.mProperty == property)
			{
				return index;
			}
		}

		return -1;
	}

	bool ResourceManagerService::updateExistingObjects(const ExistingObjectMap& existingObjectMap, UnresolvedPointerList& unresolvedPointers, InitResult& initResult)
	{
		for (auto kvp : existingObjectMap)
		{
			Resource* resource = rtti_cast<Resource>(kvp.first);
			if (resource == nullptr)
				continue;

			Resource* existing_resource = rtti_cast<Resource>(kvp.second);
			assert(existing_resource != nullptr);

			if (!initResult.check(existing_resource->get_type() == resource->get_type(), "Unable to update object, different types"))		// todo: actually support this properly
				return false;

			for (const RTTI::Property& property : resource->get_type().get_properties())
			{
				if (property.get_type().is_pointer())
				{
					int unresolved_pointer_index = findUnresolvedPointer(unresolvedPointers, resource, property);
					assert(unresolved_pointer_index != -1);

					unresolvedPointers[unresolved_pointer_index].mObject = existing_resource;
				}
				else
				{
					RTTI::Variant new_value = property.get_value(*resource);
					property.set_value(existing_resource, new_value);
				}
			}
		}

		return true;
	}

	void ResourceManagerService::backupObjects(const ExistingObjectMap& objects, ExistingObjectMap& backups)
	{
		for (auto kvp : objects)
		{
			Object* source = kvp.first;				// read object
			Object* target = kvp.second;			// object in ResourceMgr
			Object* copy = rttiCloneObject(*target);
			backups.insert({ source, copy });		// Mapping from 'read object' to backup of file in ResourceMgr
		}
	}

	void ResourceManagerService::restoreObjects(ExistingObjectMap& objects, const ExistingObjectMap& backups)
	{
		for (auto kvp : objects)
		{
			Object* source = kvp.first;
			Object* target = kvp.second;

			ExistingObjectMap::const_iterator backup = backups.find(source);
			assert(backup != backups.end());

			rttiCopyObject(*(backup->second), *target);
		}
	}


	void ResourceManagerService::rollback(ExistingObjectMap& existingObjects, const ExistingObjectMap& backupObjects, const ObjectList& newObjects)
	{
		restoreObjects(existingObjects, backupObjects);
		
		for (auto kvp : backupObjects)
		{
			delete kvp.first;		// This is the existing object as read from disk
			delete kvp.second;		// This is the backup
		}

		for (Object* object : newObjects)
		{
			nap::Resource* resource = rtti_cast<Resource>(object);
			if (resource != nullptr)
			{
				if (findResource(resource->mID) != nullptr)
					removeResource(resource->mID);
			}

			delete object;
		}
	}

	struct ObjectGraphNode;

	enum class EObjectLinkType : uint8
	{
		Object,
		File
	};

	struct ObjectGraphEdge
	{
		EObjectLinkType mType;
		ObjectGraphNode* mSource = nullptr;
		ObjectGraphNode* mDest = nullptr;
	};

	struct ObjectGraphNode
	{
		int mDepth = -1;
		Object* mObject = nullptr;
		std::string mFile;
		std::vector<ObjectGraphEdge*> mIncomingEdges;
		std::vector<ObjectGraphEdge*> mOutgoingEdges;
	};

	class ObjectGraph
	{
	public:

		~ObjectGraph()
		{
			// TODO: destruct
		}

		bool Build(const ObjectList& objectList, InitResult& initResult)
		{
			using ObjectMap = std::map<std::string, Object*>;
			ObjectMap object_map;

			// Build map from ID => object
			for (Object* object : objectList)
			{
				object_map.insert({ object->mID, object });
			}

			for (Object* object : objectList)
			{
				ObjectGraphNode* source_node = GetOrCreateObjectNode(*object);

				std::vector<Object*> object_links;
				rttiFindObjectLinks(*object, object_links);

				for (Object* object : object_links)
				{
					ObjectMap::iterator dest_object = object_map.find(object->mID);
					if (!initResult.check(dest_object != object_map.end(), "Object %s is pointing to an object that is not in the file!", object->mID.c_str()))
						return false;

					ObjectGraphEdge* edge = new ObjectGraphEdge();
					edge->mType		= EObjectLinkType::Object;
					edge->mSource	= source_node;
					edge->mDest		= GetOrCreateObjectNode(*(dest_object->second));
					edge->mDest->mIncomingEdges.push_back(edge);
					edge->mSource->mOutgoingEdges.push_back(edge);
					mEdges.push_back(edge);
				}

				std::vector<std::string> file_links;
				rttiFindFileLinks(*object, file_links);

				for (std::string& filename : file_links)
				{
					ObjectGraphEdge* edge = new ObjectGraphEdge();
					edge->mType		= EObjectLinkType::File;
					edge->mSource	= source_node;
					edge->mDest		= GetOrCreateFileNode(filename);
					edge->mDest->mIncomingEdges.push_back(edge);
					edge->mSource->mOutgoingEdges.push_back(edge);
					mEdges.push_back(edge);
				}
			}
			
			// Assign graph depth
			std::vector<ObjectGraphNode*> root_nodes;
			for (auto kvp : mNodes)
			{
				ObjectGraphNode* node = kvp.second;
				if (node->mIncomingEdges.empty())
					root_nodes.push_back(node);
			}

			for (ObjectGraphNode* root_node : root_nodes)
			{
				assignDepthRecursive(root_node, 0);
			}

			return true;
		}

		ObjectGraphNode* GetNode(const std::string& ID)
		{
			NodeMap::iterator iter = mNodes.find(ID);
			assert(iter != mNodes.end());
			return iter->second;
		}

	private:

		void assignDepthRecursive(ObjectGraphNode* node, int depth)
		{
			if (node->mDepth >= depth)
				return;

			for (ObjectGraphEdge* outgoing_edge : node->mOutgoingEdges)
				assignDepthRecursive(outgoing_edge->mDest, depth + 1);

			node->mDepth = depth;
		}

		ObjectGraphNode* GetOrCreateObjectNode(Object& object)
		{
			ObjectGraphNode* node;
			NodeMap::iterator iter = mNodes.find(object.mID);
			if (iter == mNodes.end())
			{
				node = new ObjectGraphNode();
				node->mObject = &object;
				mNodes.insert({ object.mID, node });
			}
			else
			{
				node = iter->second;
			}

			return node;
		}

		ObjectGraphNode* GetOrCreateFileNode(const std::string& filename)
		{
			ObjectGraphNode* node;
			NodeMap::iterator iter = mNodes.find(filename);
			if (iter == mNodes.end())
			{
				node = new ObjectGraphNode();
				node->mObject = nullptr;
				node->mFile = filename;
				mNodes.insert({ filename, node });
			}
			else
			{
				node = iter->second;
			}

			return node;
		}


	private:
		using NodeMap = std::map<std::string, ObjectGraphNode*>;
		NodeMap mNodes;
		std::vector<ObjectGraphEdge*> mEdges;
	};

	static void addIncomingObjectsRecursive(ObjectGraphNode* node, std::set<ObjectGraphNode*>& objectsToInit)
	{
		if (objectsToInit.find(node) != objectsToInit.end())
			return;

		objectsToInit.insert(node);

		for (ObjectGraphEdge* incoming_edge : node->mIncomingEdges)
			addIncomingObjectsRecursive(incoming_edge->mSource, objectsToInit);
	}

	bool ResourceManagerService::determineObjectsToInit(const ExistingObjectMap& existingObjects, const ExistingObjectMap& backupObjects, const ObjectList& newObjects, const std::vector<std::string>& modifiedObjectIDs, ObjectList& objectsToInit, InitResult& initResult)
	{
		// Build an object graph of all objects in the ResourceMgr
		ObjectList all_objects;
		for (auto& kvp : mResources)
			all_objects.push_back(kvp.second.get());

		ObjectGraph object_graph;
		if (!object_graph.Build(all_objects, initResult))
			return false;

		// Build set of changed IDs. These are objects that have different attributes, and objects that are added.
		// Note: we need to use the backup objects because the original have been copied over already.
		std::set<std::string> dirty_objects;
		for (auto kvp : backupObjects)
		{
			ExistingObjectMap::const_iterator existing_object = existingObjects.find(kvp.first);
			assert(existing_object != existingObjects.end());

			if (!rttiAreObjectsEqual(*existing_object->second, *kvp.second))
				dirty_objects.insert(kvp.first->mID);
		}

		for (Object* new_object : newObjects)
			dirty_objects.insert(new_object->mID);

		for (const std::string& modified_object : modifiedObjectIDs)
			dirty_objects.insert(modified_object);

		std::set<ObjectGraphNode*> objects_to_init;
		for (const std::string dirty_object : dirty_objects)
		{
			ObjectGraphNode* node = object_graph.GetNode(dirty_object);
			addIncomingObjectsRecursive(node, objects_to_init);
		}

		std::vector<ObjectGraphNode*> sorted_objects_to_init;
		for (ObjectGraphNode* object_to_init : objects_to_init)
			sorted_objects_to_init.push_back(object_to_init);

		std::sort(sorted_objects_to_init.begin(), sorted_objects_to_init.end(),
				[](ObjectGraphNode* nodeA, ObjectGraphNode* nodeB) { return nodeA->mDepth > nodeB->mDepth; });

		for (ObjectGraphNode* sorted_object_to_init : sorted_objects_to_init)
			if (sorted_object_to_init->mObject != nullptr)
				objectsToInit.push_back(sorted_object_to_init->mObject);

		return true;
	}

	bool ResourceManagerService::loadFile(const std::string& filename, nap::InitResult& initResult)
	{
		std::vector<std::string> modified_object_ids;
		return loadFile(filename, modified_object_ids, initResult);
	}

	bool ResourceManagerService::loadFile(const std::string& filename, const std::vector<std::string>& modifiedObjectIDs, nap::InitResult& initResult)
	{
		ObjectList read_objects;
		UnresolvedPointerList unresolved_pointers;
		std::vector<FileLink> linkedFiles;
		
		// Read objects from disk into 'read_objects'. If this call fails, any objects that were 
		// successfully read are destructed, so no further action is required.
		if (!readJSonFile(filename, read_objects, linkedFiles, unresolved_pointers, initResult))
			return false;

		ExistingObjectMap existing_objects;		// Mapping from 'read object' to 'existing object in ResourceMgr'
		ObjectList new_objects;					// Objects not (yet) present in ResourceMgr
		ObjectList target_objects;				// List of all objects as they eventually will be in ResourceMgr
		splitObjects(read_objects, target_objects, existing_objects, new_objects);

		// First make clones of objects so that we can restore them if errors occurs
		ExistingObjectMap backup_objects;
		backupObjects(existing_objects, backup_objects);

		// Update attributes of objects already existing in ResourceMgr
		if (!updateExistingObjects(existing_objects, unresolved_pointers, initResult))
		{
			rollback(existing_objects, backup_objects, new_objects);
			return false;
		}

		// Add objects that were not yet present in ResourceMgr
		for (Object* object : new_objects)
		{
			nap::Resource* resource = rtti_cast<Resource>(object);
			if (resource == nullptr)
				continue;

			addResource(resource->mID, resource);
		}

		// Resolve all unresolved pointers against the ResourceMgr
		for (const UnresolvedPointer& unresolved_pointer : unresolved_pointers)
		{
			nap::Resource* source_resource = rtti_cast<Resource>(unresolved_pointer.mObject);
			if (source_resource == nullptr)
				continue;

			Resource* target_resource = findResource(unresolved_pointer.mTargetID);
			if (!initResult.check(target_resource != nullptr, "Unable to resolve link to object %s from attribute %s", unresolved_pointer.mTargetID.c_str(), unresolved_pointer.mProperty.get_name().data()))
			{
				rollback(existing_objects, backup_objects, new_objects);
				return false;
			}

 			bool succeeded = unresolved_pointer.mProperty.set_value(unresolved_pointer.mObject, target_resource);
 			if (!initResult.check(succeeded, "Failed to resolve pointer"))
			{
				rollback(existing_objects, backup_objects, new_objects);
				return false;
			}
		}

		ObjectList objects_to_init;
		if (!determineObjectsToInit(existing_objects, backup_objects, new_objects, modifiedObjectIDs, objects_to_init, initResult))
		{
			rollback(existing_objects, backup_objects, new_objects);
			return false;
		}
		
		// Init all objects
		std::vector<Resource*> initted_objects;
		bool init_success = true;
		for (Object* object : objects_to_init)
		{
			nap::Resource* resource = rtti_cast<Resource>(object);
			if (resource == nullptr)
				continue;

			initted_objects.push_back(resource);

			if (!resource->init(initResult))
			{
				init_success = false;
				break;
			}
		}

		// In case of error, rollback all modifications done by attempted init calls
		if (!init_success)
		{
			for (Resource* initted_object : initted_objects)
				initted_object->finish(Resource::EFinishMode::ROLLBACK);

			rollback(existing_objects, backup_objects, new_objects);
			return false;
		}

		// Everything successful, commit changes
		for (Object* object : target_objects)
		{
			nap::Resource* resource = rtti_cast<Resource>(object);
			if (resource == nullptr)
				continue;

			resource->finish(Resource::EFinishMode::COMMIT);
		}

		for (const FileLink& file_link : linkedFiles)
		{
			addFileLink(FileLinkSource(filename, file_link.mSourceObjectID), file_link.mTargetFile);
		}

		mFilesToWatch.insert(toComparableFilename(filename));

		return true;
	}

	void ResourceManagerService::checkForFileChanges()
	{
		std::string modified_file;
		if (mDirectoryWatcher->update(modified_file))
		{
			modified_file = toComparableFilename(modified_file);

			std::vector<std::string> modified_objects;	// List of object marked as 'modified' because they are linking to a modified file
			std::set<std::string> files_to_reload;
			if (mFilesToWatch.find(modified_file) != mFilesToWatch.end())
			{
				files_to_reload.insert(modified_file);
			}
			else
			{
				FileLinkMap::iterator existing = mFileLinkMap.find(modified_file);
				if (existing != mFileLinkMap.end())
				{
					for (FileLinkSource& file_link_source : existing->second)
					{
						modified_objects.push_back(file_link_source.mSourceObjectID);
						files_to_reload.insert(file_link_source.mSourceFile);
					}
				}
			}

			if (!files_to_reload.empty())
			{
				nap::Logger::info("Detected change to %s. Files needing reload:", modified_file.c_str());
				for (const std::string& source_file : files_to_reload)
					nap::Logger::info("\t-> %s", source_file.c_str());

				for (const std::string& source_file : files_to_reload)
				{
					nap::InitResult initResult;
					if (!loadFile(source_file, modified_objects, initResult))
					{
						nap::Logger::warn("Failed to reload %s: %s. See log for more information.", source_file.c_str(), initResult.mErrorString.c_str());
						break;
					}
				}
			}
		}
	}

	Resource* ResourceManagerService::findResource(const std::string& id)
	{
		const auto& it = mResources.find(id);
		
		if (it != mResources.end())
			return it->second.get();
		
		return nullptr;
	}

	void ResourceManagerService::addResource(const std::string& id, Resource* resource)
	{
		assert(mResources.find(id) == mResources.end());
		mResources.emplace(id, std::move(std::unique_ptr<Resource>(resource)));
	}

	void ResourceManagerService::removeResource(const std::string& id)
	{
		assert(mResources.find(id) != mResources.end());
		mResources.erase(mResources.find(id));
	}

	void ResourceManagerService::addFileLink(FileLinkSource source, const std::string& targetFile)
	{
		source.mSourceFile = toComparableFilename(source.mSourceFile);
		std::string target_file = toComparableFilename(targetFile);
		
		FileLinkMap::iterator existing = mFileLinkMap.find(targetFile);
		if (existing == mFileLinkMap.end())
		{
			std::vector<FileLinkSource> source_files;
			source_files.push_back(source);

			mFileLinkMap.insert({ target_file, source_files });
		}
		else
		{
			existing->second.push_back(source);
		}
	}

	Resource* ResourceManagerService::createResource(const RTTI::TypeInfo& type)
	{
		if (!type.is_derived_from(RTTI_OF(Resource)))
		{
			nap::Logger::warn("unable to create resource of type: %s", type.get_name().data());
			return nullptr;
		}

		if (!type.can_create_instance())
		{
			nap::Logger::warn("can't create resource instance of type: %s", type.get_name().data());
			return nullptr;
		}

		// Create instance of resource
		Resource* resource = type.create<Resource>();

		// Construct path
		std::string type_name = type.get_name().data();
		std::string reso_path = stringFormat("resource::%s", type_name.c_str());
		std::string reso_unique_path = reso_path;
		int idx = 0;
		while (mResources.find(reso_unique_path) != mResources.end())
		{
			++idx;
			reso_unique_path = stringFormat("%s_%d", reso_path.c_str(), idx);
		}

		resource->mID = reso_unique_path;
		addResource(reso_unique_path, resource);
		
		return resource;
	}
}