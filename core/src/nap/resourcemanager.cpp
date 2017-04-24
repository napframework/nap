#include "resourcemanager.h"
#include "rttiutilities.h"
#include "directorywatcher.h"
#include "jsonreader.h"


RTTI_DEFINE(nap::ResourceManagerService)

namespace nap
{

	/**
	* Helper to find index into unresolved pointer array.
	*/
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


	/** 
	* Scans objects for links to other objects and other files. This is done through RTTI scanning, where
	* link properties are examined. The end result is a list of Nodes and Edges that can be used to traverse
	* the graph.
	*/
	class ObjectGraph final
	{
	public:
		enum class EEdgeType : uint8
		{
			Object,
			File
		};

		struct Node;

		/**
		* Represent an 'edge' in the ObjectGraph. This is a link from an object to either a file or another object.
		*/
		struct Edge
		{
			EEdgeType	mType;					// File/Object target
			Node*		mSource = nullptr;		// Link source
			Node*		mDest = nullptr;		// Link target
		};


		/*
		* Represents a node in the ObjectGraph, which can be an object or a file.
		*/
		struct Node
		{
			int					mDepth = -1;			// Depth of the node is calculated during build, it represents at what level from the root this node is.
			Object*				mObject = nullptr;		// If this is an Object node, set to the Object, otherwise null.
			std::string			mFile;					// If this is a file node, set to the filename, otherwise empty.
			std::vector<Edge*>	mIncomingEdges;			// List of incoming edges
			std::vector<Edge*>	mOutgoingEdges;			// List of outgoing edges
		};


		/*
		* Frees all memory allocated by Build.
		*/
		~ObjectGraph()
		{
			for (auto kvp : mNodes)
				delete kvp.second;

			for (Edge* edge : mEdges)
				delete edge;
		}


		/*
		* Builds the object graph. If building fails, return false. 
		* @param objectList : list of objects to build the graph from.
		* @param initResult: if false is returned, contains error information.
		*/
		bool Build(const ObjectList& objectList, InitResult& initResult)
		{
			using ObjectMap = std::map<std::string, Object*>;
			ObjectMap object_map;

			// Build map from ID => object
			for (Object* object : objectList)
			{
				object_map.insert({ object->mID, object });
			}

			// Scan all objects for rtti links and build data structure
			for (Object* object : objectList)
			{
				Node* source_node = GetOrCreateObjectNode(*object);

				// Process pointers to other objects
				std::vector<Object*> object_links;
				rttiFindObjectLinks(*object, object_links);

				for (Object* object : object_links)
				{
					ObjectMap::iterator dest_object = object_map.find(object->mID);
					if (!initResult.check(dest_object != object_map.end(), "Object %s is pointing to an object that is not in the objectlist!", object->mID.c_str()))
						return false;

					Edge* edge = new Edge();
					edge->mType = EEdgeType::Object;
					edge->mSource = source_node;
					edge->mDest = GetOrCreateObjectNode(*(dest_object->second));
					edge->mDest->mIncomingEdges.push_back(edge);
					edge->mSource->mOutgoingEdges.push_back(edge);
					mEdges.push_back(edge);
				}
			
				// Process pointers to files
				std::vector<std::string> file_links;
				rttiFindFileLinks(*object, file_links);

				for (std::string& filename : file_links)
				{
					Edge* edge = new Edge();
					edge->mType = EEdgeType::File;
					edge->mSource = source_node;
					edge->mDest = GetOrCreateFileNode(filename);
					edge->mDest->mIncomingEdges.push_back(edge);
					edge->mSource->mOutgoingEdges.push_back(edge);
					mEdges.push_back(edge);
				}
			}

			// Assign graph depth
			std::vector<Node*> root_nodes;
			for (auto kvp : mNodes)
			{
				Node* node = kvp.second;
				if (node->mIncomingEdges.empty())
					root_nodes.push_back(node);
			}

			for (Node* root_node : root_nodes)
			{
				assignDepthRecursive(root_node, 0);
			}

			return true;
		}


		/*
		* Returns object graph node.
		*/
		Node* GetNode(const std::string& ID)
		{
			NodeMap::iterator iter = mNodes.find(ID);
			assert(iter != mNodes.end());
			return iter->second;
		}

	private:

		/**
		* Recursively scans outgoing edges of nodes and increments the depth for each level deeper.
		* We try to avoid rescanning parts of a graph that have already been processed, but sometimes
		* we must revisit a part of the graph to make sure that the depth is accurate. For example:
		*
		*	A ---> B ----------> C
		*    \                /
		*     \---> D ---> E -
		*
		* In this situation, if the A-B-C branch is visited first, C will have depth 3, but it will
		* be corrected when processing branch A-D-E-C: object E will have depth 3, which is >= object C's depth.
		* So, the final depth will become:
		* 
		*	A(1) ---> B(2) ----------> C(4)
		*    \                      /
		*     \---> D(2) ---> E(3) -
		*
		*/
		void assignDepthRecursive(Node* node, int depth)
		{
			// The following is both a test for 'is visited' and 'should we revisit':
			if (node->mDepth >= depth)
				return;

			for (Edge* outgoing_edge : node->mOutgoingEdges)
				assignDepthRecursive(outgoing_edge->mDest, depth + 1);

			node->mDepth = depth;
		}


		/**
		* Creates a node of the 'object node' type.
		*/
		Node* GetOrCreateObjectNode(Object& object)
		{
			Node* node;
			NodeMap::iterator iter = mNodes.find(object.mID);
			if (iter == mNodes.end())
			{
				node = new Node();
				node->mObject = &object;
				mNodes.insert({ object.mID, node });
			}
			else
			{
				node = iter->second;
			}

			return node;
		}


		/**
		* Creates a node of the 'file node' type.
		*/
		Node* GetOrCreateFileNode(const std::string& filename)
		{
			Node* node;
			NodeMap::iterator iter = mNodes.find(filename);
			if (iter == mNodes.end())
			{
				node = new Node();
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
		using NodeMap = std::map<std::string, Node*>;
		NodeMap				mNodes;		// All nodes in the graph, mapped from ID to node
		std::vector<Edge*>	mEdges;		// All edges in the graph
	};


	/**
	* Walks object graph by traversing incoming edges and pushing the results in an array.
	* @param node: start node to traverse incoming edges from.
	* @param incomingObjects: output of the function.
	*/
	void addIncomingObjectsRecursive(ObjectGraph::Node* node, std::set<ObjectGraph::Node*>& incomingObjects)
	{
		if (incomingObjects.find(node) != incomingObjects.end())
			return;

		incomingObjects.insert(node);

		for (ObjectGraph::Edge* incoming_edge : node->mIncomingEdges)
			addIncomingObjectsRecursive(incoming_edge->mSource, incomingObjects);
	}


	//////////////////////////////////////////////////////////////////////////
	// ObjectRestorer
	//////////////////////////////////////////////////////////////////////////

	/*
	* This is a helper object for the loading of objects that has two purposes:
	* 1) To capture state and perform a rollback of state on destruction.
	* 2) To perform memory management of state.
	*
	* On init(), objects are separated into lists that determine whether objects already existed in the manager.
	* for objects that already existed, a clone is created.
	* On destruction, the following things take place:
	*	- If 'restore' is enabled, objects from mExistingObjectMap are restored by copying rtti attributes from the clone back into the source object.
	*	- If 'restore' is enabled, Objects from the mNewObjects array will be removed from the manager (if they exist).
	*	- File objects are destructed
	*	- Cloned objects are destructed
	* To disable restoring of the objects, use enableRestore(false).
	*/
	class ObjectRestorer final
	{
	public:

		ObjectRestorer(ResourceManagerService& resourceManagerService) :
			mResourceManagerService(resourceManagerService)
		{
		}


		ObjectRestorer::~ObjectRestorer()
		{
			if (mRestore)
			{
				// Copy attributes from clones back to the original objects
				for (auto kvp : mExistingObjects)
				{
					Object* source = kvp.first;
					Object* target = kvp.second;

					ExistingObjectMap::const_iterator backup = mClonedObjects.find(source);
					assert(backup != mClonedObjects.end());

					rttiCopyObject(*(backup->second), *target);
				}

				// Remove objects from resource manager if they were added
				for (Object* object : mNewObjects)
				{
					nap::Resource* resource = rtti_cast<Resource>(object);
					if (resource != nullptr)
					{
						if (mResourceManagerService.findResource(resource->mID) != nullptr)
							mResourceManagerService.removeResource(resource->mID);
					}
				}

				// Delete all file objects
				for (Object* file_object : mFileObjects)
					delete file_object;

				// Delete clones
				for (auto kvp : mClonedObjects)
					delete kvp.second;

			}
			else
			{
				// Destroy all clones and the file objects of the existing objects.
				// Notice that we do not touch the file objects of the new objects, as they 
				// are added to the resource manager!
				for (auto kvp : mClonedObjects)
				{
					delete kvp.first;		// This is the file object of the existing object
					delete kvp.second;		// This is the backup
				}
			}
		}


		/**
		* Splits file objects into existing/new/target lists. Clones existing objects.
		*/
		void processFileObjects()
		{
			// Split file objects into new/existing/target lists
			for (Object* source_object : mFileObjects)
			{
				Resource* resource = rtti_cast<Resource>(source_object);
				if (resource == nullptr)
					continue;

				std::string id = resource->mID;

				Resource* existing_resource = mResourceManagerService.findResource(id);
				if (existing_resource == nullptr)
				{
					mNewObjects.push_back(source_object);
					mTargetObjects.push_back(source_object);
				}
				else
				{
					mExistingObjects.insert({ source_object, existing_resource });
					mTargetObjects.push_back(existing_resource);
				}
			}

			// Make backups of all existing objects by cloning them
			for (auto kvp : mExistingObjects)
			{
				Object* source = kvp.first;					// read object
				Object* target = kvp.second;				// object in ResourceMgr
				Object* copy = rttiCloneObject(*target);
				mClonedObjects.insert({ source, copy });	// Mapping from 'read object' to backup of file in ResourceMgr
			}
		}


		/**
		* Enables or disables restoring of attributes on destruction. Default is set to true.
		*/
		void enableRestore(bool inRestore) 
		{ 
			mRestore = inRestore; 
		}


		/**
		* Returns objects cloned from existing objects during init().
		*/
		const ExistingObjectMap& getClonedObjects() const 
		{ 
			return mClonedObjects; 
		}


		/**
		* Returns objects existing in the resource manager, as determined during init().
		*/
		ExistingObjectMap& getExistingObjects() 
		{ 
			return mExistingObjects; 
		}


		/**
		* Returns objects as read from file.
		*/
		ObjectList& getFileObjects() 
		{ 
			return mFileObjects; 
		}


		/**
		* Returns objects not present in the resource manager, as determined during init().
		*/
		ObjectList& getNewObjects() 
		{ 
			return mNewObjects; 
		}


		/**
		* Returns target of the object as it will eventually be in the manager: for existing objects,
		* this is not the file object, but the file already existing in the manager. For new objects,
		* it is the same object as read from the file.
		*/
		ObjectList& getTargetObjects() { return mTargetObjects; }

	private:
		ResourceManagerService&		mResourceManagerService;
		ObjectList					mFileObjects;				// Objects as read from file
		ExistingObjectMap			mExistingObjects;			// Mapping from 'file object' to 'existing object in ResourceMgr'
		ObjectList					mNewObjects;				// Objects not (yet) present in ResourceMgr
		ObjectList					mTargetObjects;				// List of all objects as they eventually will be in ResourceMgr
		ExistingObjectMap			mClonedObjects;				// Objects as cloned from existing objects during init()
		bool						mRestore = true;			// Flag indicating whether to enable or disable restoring during destruction
	};


	//////////////////////////////////////////////////////////////////////////
	// ResourceManagerService
	//////////////////////////////////////////////////////////////////////////


	ResourceManagerService::ResourceManagerService() :
		mDirectoryWatcher(new DirectoryWatcher())
	{
	}


	/**
	* Copies rtti attributes from File object to object existing in the manager. Patches the unresolved pointer list so the it contains the correct object.
	*/
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
					// Patch the mObject member: it should not use the File object anymore, but the object from the manager
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


	/**
	* Builds an object graph of all objects currently in the manager. Then, from all objects that are effectively changed (determined by an RTTI diff), it traverses 
	* the object graph to find the minimum set of objects that requires an init. Finally, the list of objects is sorted on object graph depth so that the init() order
	* is correct.
	*/
	bool ResourceManagerService::determineObjectsToInit(const ExistingObjectMap& existingObjects, const ExistingObjectMap& clonedObjects, const ObjectList& newObjects, const std::vector<std::string>& modifiedObjectIDs, ObjectList& objectsToInit, InitResult& initResult)
	{
		// Build an object graph of all objects in the ResourceMgr
		ObjectList all_objects;
		for (auto& kvp : mResources)
			all_objects.push_back(kvp.second.get());

		ObjectGraph object_graph;
		if (!object_graph.Build(all_objects, initResult))
			return false;

		// Build set of changed IDs. These are objects that have different attributes, and objects that are added.
		// Note: we need to use the cloned objects because the original have been copied over already.
		std::set<std::string> dirty_objects;
		for (auto kvp : clonedObjects)
		{
			ExistingObjectMap::const_iterator existing_object = existingObjects.find(kvp.first);
			assert(existing_object != existingObjects.end());

			if (!rttiAreObjectsEqual(*existing_object->second, *kvp.second))
				dirty_objects.insert(kvp.first->mID);
		}

		// All new objects need an init
		for (Object* new_object : newObjects)
			dirty_objects.insert(new_object->mID);

		// Any objects that are passed in as 'dirty' from the outside need to have their init as well
		// These are currently objects that point to a file that is changed
		for (const std::string& modified_object : modifiedObjectIDs)
			dirty_objects.insert(modified_object);

		// Traverse graph for incoming links and add all of them
		std::set<ObjectGraph::Node*> objects_to_init;
		for (const std::string dirty_object : dirty_objects)
		{
			ObjectGraph::Node* node = object_graph.GetNode(dirty_object);
			addIncomingObjectsRecursive(node, objects_to_init);
		}

		// Sort on graph depth for the correct init() order
		std::vector<ObjectGraph::Node*> sorted_objects_to_init;
		for (ObjectGraph::Node* object_to_init : objects_to_init)
			sorted_objects_to_init.push_back(object_to_init);

		std::sort(sorted_objects_to_init.begin(), sorted_objects_to_init.end(),
				[](ObjectGraph::Node* nodeA, ObjectGraph::Node* nodeB) { return nodeA->mDepth > nodeB->mDepth; });

		for (ObjectGraph::Node* sorted_object_to_init : sorted_objects_to_init)
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
		UnresolvedPointerList unresolved_pointers;
		std::vector<FileLink> linkedFiles;

		ObjectRestorer object_restorer(*this);

		// Read objects from disk into 'read_objects'. If this call fails, any objects that were 
		// successfully read are destructed, so no further action is required.
		if (!readJSonFile(filename, object_restorer.getFileObjects(), linkedFiles, unresolved_pointers, initResult))
			return false;

		// Split the read objects into separate lists and clone the existing objects
		object_restorer.processFileObjects();

		// Update attributes of objects already existing in ResourceMgr
		if (!updateExistingObjects(object_restorer.getExistingObjects(), unresolved_pointers, initResult))
			return false;

		// Add objects that were not yet present in ResourceMgr
		for (Object* object : object_restorer.getNewObjects())
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
				return false;

 			bool succeeded = unresolved_pointer.mProperty.set_value(unresolved_pointer.mObject, target_resource);
 			if (!initResult.check(succeeded, "Failed to resolve pointer"))
				return false;
		}

		// Find out what objects to init and in what order to init them
		ObjectList objects_to_init;
		if (!determineObjectsToInit(object_restorer.getExistingObjects(), object_restorer.getClonedObjects(), object_restorer.getNewObjects(), modifiedObjectIDs, objects_to_init, initResult))
			return false;
		
		// Init all objects in the correct order
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

			return false;
		}

		// Everything was successful. Tell the restorer not to perform a rollback of the state.
		object_restorer.enableRestore(false);

		// Everything successful, commit changes
		for (Object* object : object_restorer.getTargetObjects())
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