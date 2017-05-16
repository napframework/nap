#include "resourcemanager.h"
#include "rtti/rttiutilities.h"
#include "directorywatcher.h"
#include "rtti/jsonreader.h"
#include "rtti/factory.h"


RTTI_DEFINE(nap::ResourceManagerService)

namespace nap
{
	using namespace rtti;

	/**
	* Helper to find index into unresolved pointer array.
	*/
	static int findUnresolvedPointer(UnresolvedPointerList& unresolvedPointers, RTTIObject* object, const rtti::RTTIPath& path)
	{
		for (int index = 0; index < unresolvedPointers.size(); ++index)
		{
			UnresolvedPointer& unresolved_pointer = unresolvedPointers[index];
			if (unresolved_pointer.mObject == object &&
				unresolved_pointer.mRTTIPath == path)
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
			RTTIObject*			mObject = nullptr;		// If this is an Object node, set to the Object, otherwise null.
			std::string			mFile;					// If this is a file node, set to the filename, otherwise empty.
			std::vector<Edge*>	mIncomingEdges;			// List of incoming edges
			std::vector<Edge*>	mOutgoingEdges;			// List of outgoing edges
		};


		/*
		* Builds the object graph. If building fails, return false. 
		* @param objectList : list of objects to build the graph from.
		* @param errorState: if false is returned, contains error information.
		*/
		bool Build(const ObservedObjectList& objectList, utility::ErrorState& errorState)
		{
			using ObjectMap = std::map<std::string, RTTIObject*>;
			ObjectMap object_map;

			// Build map from ID => object
			for (RTTIObject* object : objectList)
			{
				object_map.insert({ object->mID, object });
			}

			// Scan all objects for rtti links and build data structure
			for (RTTIObject* object : objectList)
			{
				Node* source_node = GetOrCreateObjectNode(*object);

				// Process pointers to other objects
				std::vector<rtti::ObjectLink> object_links;
				rtti::findObjectLinks(*object, object_links);

				for (const rtti::ObjectLink& link : object_links)
				{
					RTTIObject* linked_object = link.mTarget;

					ObjectMap::iterator dest_object = object_map.find(linked_object->mID);
					if (!errorState.check(dest_object != object_map.end(), "Object %s is pointing to an object that is not in the objectlist!", linked_object->mID.c_str()))
						return false;

					Edge* edge = new Edge();
					edge->mType = EEdgeType::Object;
					edge->mSource = source_node;
					edge->mDest = GetOrCreateObjectNode(*(dest_object->second));
					edge->mDest->mIncomingEdges.push_back(edge);
					edge->mSource->mOutgoingEdges.push_back(edge);
					mEdges.push_back(std::unique_ptr<Edge>(edge));
				}
			
				// Process pointers to files
				std::vector<std::string> file_links;
				rtti::findFileLinks(*object, file_links);

				for (std::string& filename : file_links)
				{
					Edge* edge = new Edge();
					edge->mType = EEdgeType::File;
					edge->mSource = source_node;
					edge->mDest = GetOrCreateFileNode(filename);
					edge->mDest->mIncomingEdges.push_back(edge);
					edge->mSource->mOutgoingEdges.push_back(edge);
					mEdges.push_back(std::unique_ptr<Edge>(edge));
				}
			}

			// Assign graph depth
			std::vector<Node*> root_nodes;
			for (auto& kvp : mNodes)
			{
				Node* node = kvp.second.get();
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
		Node* FindNode(const std::string& ID)
		{
			NodeMap::iterator iter = mNodes.find(ID);
			if (iter == mNodes.end())
				return nullptr;
			
			return iter->second.get();
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
		Node* GetOrCreateObjectNode(RTTIObject& object)
		{
			Node* result = nullptr;
			NodeMap::iterator iter = mNodes.find(object.mID);
			if (iter == mNodes.end())
			{
                auto node = std::make_unique<Node>();
                node->mObject = &object;
                result = node.get();
                mNodes.insert(std::make_pair(object.mID, std::move(node)));
			}
			else
			{
				result = iter->second.get();
			}

            return result;
		}


		/**
		* Creates a node of the 'file node' type.
		*/
		Node* GetOrCreateFileNode(const std::string& filename)
		{
            Node* result = nullptr;
			NodeMap::iterator iter = mNodes.find(filename);
			if (iter == mNodes.end())
			{
                auto node = std::make_unique<Node>();
                node->mObject = nullptr;
                node->mFile = filename;
                result = node.get();
                mNodes.insert(std::make_pair(filename, std::move(node)));
			}
			else
			{
				result = iter->second.get();
			}

			return result;
		}

	private:
		using NodeMap = std::map<std::string, std::unique_ptr<Node>>;
		NodeMap								mNodes;		// All nodes in the graph, mapped from ID to node
		std::vector<std::unique_ptr<Edge>>	mEdges;		// All edges in the graph
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
	* This is a helper object to restore state during loading. On construction,
	* an rtti clone is created of all the 'existing' objects. On destruction, the 
	* clone is used to copy the rtti attribute values back from the clone into the
	* original object. Also, any 'new' objects that were added to the manager are
	* removed (destroying them in the process).
	* To disable restoring of the objects, use enableRestore(false).
	*/
	class ObjectRestorer final
	{
	public:

		ObjectRestorer(ResourceManagerService& resourceManagerService, ResourceManagerService::ExistingObjectMap& existingObjects, ObservedObjectList& newObjects) :
			mResourceManagerService(resourceManagerService),
			mExistingObjects(existingObjects),
			mNewObjects(newObjects)
		{
			// Make backups of all existing objects by cloning them
			for (auto kvp : mExistingObjects)
			{
				RTTIObject* source = kvp.first;			// file object
				RTTIObject* target = kvp.second;		// object in ResourceMgr
				std::unique_ptr<RTTIObject> copy = std::move(rtti::cloneObject(*target));
                mClonedObjects[source] = std::move(copy); // Mapping from 'read object' to backup of file in ResourceMgr
			}
		}


		~ObjectRestorer()
		{
			if (mRestore)
			{
				// Copy attributes from clones back to the original objects
				for (auto kvp : mExistingObjects)
				{
					RTTIObject* source = kvp.first;
					RTTIObject* target = kvp.second;

					ResourceManagerService::ClonedObjectMap::const_iterator clone = mClonedObjects.find(source);
					assert(clone != mClonedObjects.end());

					rtti::copyObject(*(clone->second), *target);
				}

				// Remove objects from resource manager if they were added
				for (auto& object : mNewObjects)
				{
					nap::Resource* resource = rtti_cast<Resource>(object);
					if (resource != nullptr)
					{
						if (mResourceManagerService.findResource(resource->mID) != nullptr)
							mResourceManagerService.removeResource(resource->mID);
					}
				}
			}
		}


		/**
		* Disables restoring of attributes on destruction.
		*/
		void clear()
		{ 
			mRestore = false; 
		}


		/**
		* Returns objects cloned from existing objects during init().
		*/
		const ResourceManagerService::ClonedObjectMap& getClonedObjects() const
		{ 
			return mClonedObjects; 
		}

	private:
		ResourceManagerService&							mResourceManagerService;
		ResourceManagerService::ClonedObjectMap			mClonedObjects;				// Objects as cloned from existing objects during init()
		ResourceManagerService::ExistingObjectMap&		mExistingObjects;			// Objects existing in manager
		ObservedObjectList&								mNewObjects;				// Objects as added to the manager during loading
		bool											mRestore = true;			// Flag indicating whether to enable or disable restoring during destruction
	};


	//////////////////////////////////////////////////////////////////////////
	// ResourceManagerService
	//////////////////////////////////////////////////////////////////////////


	ResourceManagerService::ResourceManagerService() :
		mDirectoryWatcher(std::make_unique<DirectoryWatcher>()),
		mFactory(std::make_unique<Factory>())
	{
	}


	/**
	* Copies rtti attributes from File object to object existing in the manager. Patches the unresolved pointer list so the it contains the correct object.
	*/
	bool ResourceManagerService::updateExistingObjects(const ExistingObjectMap& existingObjectMap, UnresolvedPointerList& unresolvedPointers, utility::ErrorState& errorState)
	{
		for (auto kvp : existingObjectMap)
		{
			Resource* resource = rtti_cast<Resource>(kvp.first);
			if (resource == nullptr)
				continue;

			Resource* existing_resource = rtti_cast<Resource>(kvp.second);
			assert(existing_resource != nullptr);

			if (!errorState.check(existing_resource->get_type() == resource->get_type(), "Unable to update object, different types"))		// todo: actually support this properly
				return false;

			// Find all links from the resource
			std::vector<rtti::ObjectLink> links;
			rtti::findObjectLinks(*resource, links);

			// We need to update the UnresolvedPointers to by unresolved against the object from the manager, instead of the object from the file
			for (const rtti::ObjectLink& link : links)
			{
				// Patch the mObject member: it should not use the File object anymore, but the object from the manager
				int unresolved_pointer_index = findUnresolvedPointer(unresolvedPointers, resource, link.mSourcePath);
				assert(unresolved_pointer_index != -1);
				unresolvedPointers[unresolved_pointer_index].mObject = existing_resource;
			}

			// Copy regular properties
			for (const rtti::Property& property : resource->get_type().get_properties())
			{
				if (!property.get_type().is_pointer())
				{
					rtti::Variant new_value = property.get_value(*resource);
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
	bool ResourceManagerService::determineObjectsToInit(const ExistingObjectMap& existingObjects, const ClonedObjectMap& clonedObjects, const ObservedObjectList& newObjects, const std::string& externalChangedFile, ObservedObjectList& objectsToInit, utility::ErrorState& errorState)
	{
		// Build an object graph of all objects in the ResourceMgr
		ObservedObjectList all_objects;
		for (auto& kvp : mResources)
			all_objects.push_back(kvp.second.get());

		ObjectGraph object_graph;
		if (!object_graph.Build(all_objects, errorState))
			return false;

		// Build set of changed IDs. These are objects that have different attributes, and objects that are added.
		// Note: we need to use the cloned objects because the original have been copied over already.
		std::set<std::string> dirty_nodes;
		for (auto& kvp : clonedObjects)
		{
			ExistingObjectMap::const_iterator existing_object = existingObjects.find(kvp.first);
			assert(existing_object != existingObjects.end());

			if (!rtti::areObjectsEqual(*existing_object->second, *kvp.second.get()))
				dirty_nodes.insert(kvp.first->mID);
		}

		// All new objects need an init
		for (RTTIObject* new_object : newObjects)
			dirty_nodes.insert(new_object->mID);

		// Add externally changed file that caused load of this json file
		if (!externalChangedFile.empty())
			dirty_nodes.insert(externalChangedFile);

		// Traverse graph for incoming links and add all of them
		std::set<ObjectGraph::Node*> objects_to_init;
		for (const std::string& dirty_node : dirty_nodes)
		{
			ObjectGraph::Node* node = object_graph.FindNode(dirty_node);
		
			// In the case that file links change as part of the file modification(s), it's possible for the dirty node to not be present in the ObjectGraph,
			// so we can't assert here but need to deal with that case.
			if (node != nullptr)
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

	
	/** 
	* Splits fileObjects into lists of objects that exist in the manager and do not yet exist.
	* @param fileObjects: objects as read from file.
	* @param existingObjects: mapping from file object to object in manager
	* @param newObjects: list of objects not yet in manager.
	*/
	void ResourceManagerService::splitFileObjects(OwnedObjectList& fileObjects, ExistingObjectMap& existingObjects, ObservedObjectList& newObjects)
	{
		// Split file objects into new/existing/target lists
		for (auto& source_object : fileObjects)
		{
			Resource* resource = rtti_cast<Resource>(source_object.get());
			if (resource == nullptr)
				continue;

			std::string id = resource->mID;

			Resource* existing_resource = findResource(id);
			if (existing_resource == nullptr)
				newObjects.push_back(source_object.get());
			else
				existingObjects.insert({ source_object.get(), existing_resource });
		}
	}


	bool ResourceManagerService::loadFile(const std::string& filename, utility::ErrorState& errorState)
	{
		return loadFile(filename, std::string(), errorState);
	}


	/*
	* Important to understand in this function is how ownership flows through the various data structures.
	* First, objects are loaded into file_objects and owned by it. Any objects that did not yet exist
	* in the manager are moved to the manager (so ownership is transfered for those objects, but those
	* objects only).
	* Existing objects are copied over from the file object to the existing object, and the source file object
	* will be deleted when file_objects goes out of scope, as ownership will remain in the file_objects list for
	* the existing objects.
	* When a rollback occurs, any newly added objects are removed from the manager, effectively deleting them.
	* Other maps/list like existing/new are merely observers into the file objects and manager objects.
	*/
	bool ResourceManagerService::loadFile(const std::string& filename, const std::string& externalChangedFile, utility::ErrorState& errorState)
	{
		// ExternalChangedFile should only be used if it's different from the file being reloaded
		assert(toComparableFilename(filename) != toComparableFilename(externalChangedFile));

		// Read objects from disk
		RTTIDeserializeResult read_result;
		if (!readJSONFile(filename, *mFactory.get(), read_result, errorState))
			return false;

		ExistingObjectMap existing_objects;			// Mapping from 'file object' to 'existing object in ResourceMgr'. This is an observer relationship.
		ObservedObjectList new_objects;				// Objects not (yet) present in ResourceMgr.

		// Split file objects into observer lists for new/existing objects. 
		splitFileObjects(read_result.mReadObjects, existing_objects, new_objects);

		// The ObjectRestorer is capable of undoing changes that we are going to make in the loading process
		// (adding objects, updating objects).
		ObjectRestorer object_restorer(*this, existing_objects, new_objects);

		// Update attributes of objects already existing in ResourceMgr
		if (!updateExistingObjects(existing_objects, read_result.mUnresolvedPointers, errorState))
			return false;

		// Add objects that were not yet present in ResourceMgr
		// Note that we cannot iterate over new_objects as we need to transfer ownership
		// of the file object to the resource manager.
		for (auto& object : read_result.mReadObjects)
		{
			nap::Resource* resource = rtti_cast<Resource>(object.get());
			if (resource == nullptr)
				continue;

			if (findResource(resource->mID) != nullptr)
				continue;

			// Note: because the manager currently owns only Resource types, we need to release
			// the Object* here and create a new unique_ptr of type Resource.
			object.release();
			addResource(resource->mID, std::unique_ptr<Resource>(resource));
		}

		// Resolve all unresolved pointers against the ResourceMgr
		for (const UnresolvedPointer& unresolved_pointer : read_result.mUnresolvedPointers)
		{
			nap::Resource* source_resource = rtti_cast<Resource>(unresolved_pointer.mObject);
			if (source_resource == nullptr)
				continue;

			Resource* target_resource = findResource(unresolved_pointer.mTargetID);
			if (!errorState.check(target_resource != nullptr, "Unable to resolve link to object %s from attribute %s", unresolved_pointer.mTargetID.c_str(), unresolved_pointer.mRTTIPath.toString().c_str()))
				return false;

			rtti::ResolvedRTTIPath resolved_path;
			if (!errorState.check(unresolved_pointer.mRTTIPath.resolve(unresolved_pointer.mObject, resolved_path), "Failed to resolve RTTIPath %s", unresolved_pointer.mRTTIPath.toString().c_str()))
				return false;

			if (!errorState.check(target_resource->get_type().is_derived_from(resolved_path.getType()), "Failed to resolve pointer: target of pointer {%s}:%s is of the wrong type (found '%s', expected '%s')",
									unresolved_pointer.mObject->mID.c_str(), unresolved_pointer.mRTTIPath.toString().c_str(), target_resource->get_type().get_name().data(), resolved_path.getType().get_raw_type().get_name().data()))
			{
				return false;
			}				

			assert(resolved_path.getType().is_pointer());
			bool succeeded = resolved_path.setValue(target_resource);
 			if (!errorState.check(succeeded, "Failed to resolve pointer"))
				return false;
		}

		// Find out what objects to init and in what order to init them
		ObservedObjectList objects_to_init;
		if (!determineObjectsToInit(existing_objects, object_restorer.getClonedObjects(), new_objects, externalChangedFile, objects_to_init, errorState))
			return false;
		
		// Init all objects in the correct order
		std::vector<Resource*> initted_objects;
		bool init_success = true;
		for (RTTIObject* object : objects_to_init)
		{
			nap::Resource* resource = rtti_cast<Resource>(object);
			if (resource == nullptr)
				continue;

			initted_objects.push_back(resource);

			if (!resource->init(errorState))
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
		object_restorer.clear();

		// Everything successful, commit changes
		for (Resource* resource : initted_objects)
			resource->finish(Resource::EFinishMode::COMMIT);

		for (const FileLink& file_link : read_result.mFileLinks)
			addFileLink(filename, file_link.mTargetFile);

		mFilesToWatch.insert(toComparableFilename(filename));

		return true;
	}


	void ResourceManagerService::checkForFileChanges()
	{
		std::vector<std::string> modified_files;
		if (mDirectoryWatcher->update(modified_files))
		{
			for (std::string& modified_file : modified_files)
			{
				modified_file = toComparableFilename(modified_file);

				std::set<std::string> files_to_reload;

				// Is our modified file a file that was loaded by the manager?
				if (mFilesToWatch.find(modified_file) != mFilesToWatch.end())
				{
					files_to_reload.insert(modified_file);
				}
				else
				{
					// Find all the sources of this file
					FileLinkMap::iterator file_link = mFileLinkMap.find(modified_file);
					if (file_link != mFileLinkMap.end())
						for (const std::string& source_file : file_link->second)
							files_to_reload.insert(toComparableFilename(source_file));
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
							nap::Logger::warn("Failed to reload %s:\n %s. \n\n See log for more information.", source_file.c_str(), errorState.toString().c_str());
							break;
						}
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


	void ResourceManagerService::addResource(const std::string& id, std::unique_ptr<Resource> resource)
	{
		assert(mResources.find(id) == mResources.end());
		mResources.emplace(id, std::move(resource));
	}


	void ResourceManagerService::removeResource(const std::string& id)
	{
		assert(mResources.find(id) != mResources.end());
		mResources.erase(mResources.find(id));
	}


	void ResourceManagerService::addFileLink(const std::string& sourceFile, const std::string& targetFile)
	{
		std::string source_file = toComparableFilename(sourceFile);
		std::string target_file = toComparableFilename(targetFile);
		
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


	Resource* ResourceManagerService::createResource(const rtti::TypeInfo& type)
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
		Resource* resource = rtti_cast<Resource>(mFactory->create(type));

		// Construct path
		std::string type_name = type.get_name().data();
		std::string reso_path = utility::stringFormat("resource::%s", type_name.c_str());
		std::string reso_unique_path = reso_path;
		int idx = 0;
		while (mResources.find(reso_unique_path) != mResources.end())
		{
			++idx;
			reso_unique_path = utility::stringFormat("%s_%d", reso_path.c_str(), idx);
		}

		resource->mID = reso_unique_path;
		addResource(reso_unique_path, std::unique_ptr<Resource>(resource));
		
		return resource;
	}
}
