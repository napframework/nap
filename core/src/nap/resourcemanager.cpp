#include "resourcemanager.h"
#include "rtti/rttiutilities.h"
#include "directorywatcher.h"
#include "rtti/jsonreader.h"
#include "rtti/factory.h"
#include "nap/core.h"
#include "objectptr.h"
#include "entity.h"

RTTI_DEFINE(nap::ResourceManagerService)

namespace nap
{
	using namespace rtti;


	//////////////////////////////////////////////////////////////////////////
	// ResourceManagerService::RollbackHelper
	//////////////////////////////////////////////////////////////////////////


	ResourceManagerService::RollbackHelper::RollbackHelper(ResourceManagerService& service) :
		mService(service)
	{
	}

	ResourceManagerService::RollbackHelper::~RollbackHelper()
	{
		if (mPatchObjects)
			mService.patchObjectPtrs(mService.mObjects);
	}

	void ResourceManagerService::RollbackHelper::clear()
	{
		mPatchObjects = false;
	}


	//////////////////////////////////////////////////////////////////////////
	// ObjectGraph
	//////////////////////////////////////////////////////////////////////////


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
	// ResourceManagerService
	//////////////////////////////////////////////////////////////////////////


	ResourceManagerService::ResourceManagerService() :
		mDirectoryWatcher(std::make_unique<DirectoryWatcher>())
	{ 
	}


	/**
	* Builds an object graph of all objects currently in the manager, overlayed by objects that about to be updated. Then, from all objects that are effectively changed or added, it traverses
	* the object graph to find the minimum set of objects that requires an init. Finally, the list of objects is sorted on object graph depth so that the init() order
	* is correct.
	*/
	bool ResourceManagerService::determineObjectsToInit(const ObjectByIDMap& objectsToUpdate, const std::string& externalChangedFile, std::vector<std::string>& objectsToInit, utility::ErrorState& errorState)
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

		std::set<std::string> dirty_nodes;
		for (auto& kvp : objectsToUpdate)
		{
			// Mark all the objects to update as 'dirty', we need to init() those and 
			// all the objects that point to them (recursively)
			dirty_nodes.insert(kvp.first);

			// Any objects not yet in the manager are new and need to be added to the graph as well
			if (mObjects.find(kvp.first) == mObjects.end())
				all_objects.push_back(kvp.second.get());
		}

		// Add externally changed file that caused load of this json file
		if (!externalChangedFile.empty())
			dirty_nodes.insert(externalChangedFile);

		ObjectGraph object_graph;
		if (!object_graph.Build(all_objects, errorState))
			return false;

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
				objectsToInit.push_back(sorted_object_to_init->mObject->mID);

		return true;
	}


	bool ResourceManagerService::loadFile(const std::string& filename, utility::ErrorState& errorState)
	{
		return loadFile(filename, std::string(), errorState);
	}


	bool ResourceManagerService::resolvePointers(ObjectByIDMap& objectsToUpdate, const UnresolvedPointerList& unresolvedPointers, utility::ErrorState& errorState)
	{
		for (const UnresolvedPointer& unresolved_pointer : unresolvedPointers)
		{
			// Objects in objectsToUpdate have preference over the manager's objects
			RTTIObject* target_object = nullptr;
			ObjectByIDMap::iterator object_to_update = objectsToUpdate.find(unresolved_pointer.mTargetID);
			if (object_to_update == objectsToUpdate.end())
				target_object = findObject(unresolved_pointer.mTargetID).get();
			else
				target_object = object_to_update->second.get();

			if (!errorState.check(target_object != nullptr, "Unable to resolve link to object %s from attribute %s", unresolved_pointer.mTargetID.c_str(), unresolved_pointer.mRTTIPath.toString().c_str()))
				return false;

			rtti::ResolvedRTTIPath resolved_path;
			if (!errorState.check(unresolved_pointer.mRTTIPath.resolve(unresolved_pointer.mObject, resolved_path), "Failed to resolve RTTIPath %s", unresolved_pointer.mRTTIPath.toString().c_str()))
				return false;

			rtti::TypeInfo resolved_path_type = resolved_path.getType();
			rtti::TypeInfo actual_type = resolved_path_type.is_wrapper() ? resolved_path_type.get_wrapped_type() : resolved_path_type;

			if (!errorState.check(target_object->get_type().is_derived_from(actual_type), "Failed to resolve pointer: target of pointer {%s}:%s is of the wrong type (found '%s', expected '%s')",
				unresolved_pointer.mObject->mID.c_str(), unresolved_pointer.mRTTIPath.toString().c_str(), target_object->get_type().get_name().data(), actual_type.get_raw_type().get_name().data()))
			{
				return false;
			}

			assert(actual_type.is_pointer());
			bool succeeded = resolved_path.setValue(target_object);
			if (!errorState.check(succeeded, "Failed to resolve pointer"))
				return false;
		}

		return true;
	}


	// inits all objects 
	bool ResourceManagerService::initObjects(std::vector<std::string> objectsToInit, ObjectByIDMap& objectsToUpdate, utility::ErrorState& errorState)
	{
		// Init all objects in the correct order
		for (const std::string& id : objectsToInit)
		{
			rtti::RTTIObject* object = nullptr;
			
			// We perform lookup by ID. Objects in objectsToUpdate have preference over the manager's objects.
			ObjectByIDMap::iterator updated_object = objectsToUpdate.find(id);
			if (updated_object != objectsToUpdate.end())
				object = updated_object->second.get();
			else
				object = findObject(id).get();

			if (!object->init(errorState))
				return false;
		}

		return true;
	}


	bool ResourceManagerService::initEntities(ObjectByIDMap& objectsToUpdate, utility::ErrorState& errorState)
	{
		// Build list of all entities we need to update. We need to use the objects in objectsToUpdate over those already in the ResourceManager
		// In essence, objectsToUpdate functions as an 'overlay' on top of the ResourceManager
		std::vector<EntityResource*> entities;
		
		// First add all EntityResources in the list of objects to update
		for (auto& kvp : objectsToUpdate)
		{
			if (kvp.second->get_type() != RTTI_OF(EntityResource))
				continue;

			entities.push_back(rtti_cast<EntityResource>(kvp.second.get()));
		}

		// Next, go through all EntityResources currently in the resource manager and add them if they're not in the list of objects to update
		for (auto& kvp : mObjects)
		{
			if (kvp.second->get_type() != RTTI_OF(EntityResource))
				continue;

			ObjectByIDMap::const_iterator object_to_update = objectsToUpdate.find(kvp.first);
			if (object_to_update == objectsToUpdate.end())
				entities.push_back(rtti_cast<EntityResource>(kvp.second.get()));
		}

		// Reinstantiate all entities
		EntityByIDMap new_entity_instances;

		using InstanceByIDMap = std::unordered_map<std::string, rtti::RTTIObject*>;
		InstanceByIDMap new_instances;
		for (EntityResource* entity_resource : entities)
		{
			std::unique_ptr<EntityInstance> entity_instance = std::make_unique<EntityInstance>(getCore());
			entity_instance->mID = mID + "_instance";
			for (auto& componentData : entity_resource->mComponents)
			{
				const rtti::TypeInfo& instance_type = componentData->getInstanceType();
				assert(instance_type.can_create_instance());

				std::unique_ptr<ComponentInstance> component_instance(instance_type.create<ComponentInstance>({ *entity_instance }));
				assert(component_instance);
				component_instance->mID = componentData->mID + "_instance";

				new_instances.insert(std::make_pair(component_instance->mID, component_instance.get()));
				entity_instance->addComponent(std::move(component_instance));
			}

			new_instances.insert(std::make_pair(entity_instance->mID, entity_instance.get()));
			new_entity_instances.emplace(std::make_pair(entity_resource->mID, std::move(entity_instance)));
		}

		for (EntityResource* entity_resource : entities)
		{
			EntityByIDMap::iterator entity_instance = new_entity_instances.find(entity_resource->mID);
			assert(entity_instance != new_entity_instances.end());

			for (ObjectPtr<EntityResource>& child_entity_resource : entity_resource->mChildren)
			{
				EntityByIDMap::iterator child_entity_instance = new_entity_instances.find(child_entity_resource->mID);
				assert(child_entity_instance != new_entity_instances.end());
				entity_instance->second->addChild(*child_entity_instance->second);
			}
		}

		for (EntityResource* entity_resource : entities)
		{
			for (auto& componentData : entity_resource->mComponents)
			{
				InstanceByIDMap::iterator pos = new_instances.find(componentData->mID + "_instance");
				assert(pos != new_instances.end());

				ComponentInstance* component_instance = rtti_cast<ComponentInstance>(pos->second);
				assert(component_instance != nullptr);
				
				if (!component_instance->init(componentData, errorState))
					return false;
			}
		}

		patchObjectPtrs(new_instances);

		// Replace entities currently in the resource manager with the new set
		mEntities = std::move(new_entity_instances);

		return true;
	}


	bool ResourceManagerService::loadFile(const std::string& filename, const std::string& externalChangedFile, utility::ErrorState& errorState)
	{
		// ExternalChangedFile should only be used if it's different from the file being reloaded
		assert(toComparableFilename(filename) != toComparableFilename(externalChangedFile));

		// Read objects from disk
		RTTIDeserializeResult read_result;
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
		if (!resolvePointers(objects_to_update, read_result.mUnresolvedPointers, errorState))
			return false;

		// We instantiate a helper that will perform a rollback of any pointer patching that we have done.
		// In case of success we clear this helper.
		// We only ever need to rollback the pointer patching, because the resource manager remains untouched
		// until the very end where we know that all init() calls have succeeded
		RollbackHelper rollback_helper(*this);

		// Patch ObjectPtrs so that they point to the updated object instead of the old object. We need to do this before determining
		// init order, otherwise a part of the graph may still be pointing to the old objects.
		patchObjectPtrs(objects_to_update);

		// Find out what objects to init and in what order to init them
		std::vector<std::string> objects_to_init;
		if (!determineObjectsToInit(objects_to_update, externalChangedFile, objects_to_init, errorState))
			return false;

		// The objects that require an init may contain objects that were not present in the file (because they are
		// pointing to objects that will be reconstructed and initted). In that case we reconstruct those objects 
		// as well by cloning them and pushing them into the objects_to_update list.
		for (const std::string& object_to_init : objects_to_init)
		{
			if (objects_to_update.find(object_to_init) == objects_to_update.end())
			{
				RTTIObject* object = findObject(object_to_init).get();
				assert(object);

				std::unique_ptr<RTTIObject> cloned_object = rtti::cloneObject(*object, getFactory());
				objects_to_update.emplace(std::make_pair(cloned_object->mID, std::move(cloned_object)));
			}
		}

		// Patch again to update pointers to objects that were cloned
		patchObjectPtrs(objects_to_update);

		// init all objects in the correct order
		if (!initObjects(objects_to_init, objects_to_update, errorState))
			return false;

		// Init all entities
		if (!initEntities(objects_to_update, errorState))
			return false;

		// In case all init() operations were successful, we can now replace the objects
		// in the manager by the new objects. This effectively destroys the old objects as well.
		for (auto& kvp : objects_to_update)
		{
			mObjects.erase(kvp.first);
			mObjects.emplace(std::make_pair(kvp.first, std::move(kvp.second)));
		}

		for (const FileLink& file_link : read_result.mFileLinks)
			addFileLink(filename, file_link.mTargetFile);

		mFilesToWatch.insert(toComparableFilename(filename));
		
		// Everything was successful, don't rollback any changes that were made
		rollback_helper.clear();

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


	nap::rtti::Factory& ResourceManagerService::getFactory()
	{
		return getCore().getFactory();
	}


	const ObjectPtr<RTTIObject> ResourceManagerService::findObject(const std::string& id)
	{
		const auto& it = mObjects.find(id);
		
		if (it != mObjects.end())
			return ObjectPtr<RTTIObject>(it->second.get());
		
		return nullptr;
	}


	void ResourceManagerService::addObject(const std::string& id, std::unique_ptr<RTTIObject> object)
	{
		assert(mObjects.find(id) == mObjects.end());
		mObjects.emplace(id, std::move(object));
	}


	void ResourceManagerService::removeObject(const std::string& id)
	{
		assert(mObjects.find(id) != mObjects.end());
		mObjects.erase(mObjects.find(id));
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


	const ObjectPtr<RTTIObject> ResourceManagerService::createObject(const rtti::TypeInfo& type)
	{
		if (!type.is_derived_from(RTTI_OF(RTTIObject)))
		{
			nap::Logger::warn("unable to create object of type: %s", type.get_name().data());
			return nullptr;
		}

		if (!type.can_create_instance())
		{
			nap::Logger::warn("can't create object instance of type: %s", type.get_name().data());
			return nullptr;
		}

		// Create instance of object
		RTTIObject* object = rtti_cast<RTTIObject>(getFactory().create(type));

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
		addObject(reso_unique_path, std::unique_ptr<RTTIObject>(object));
		
		return ObjectPtr<RTTIObject>(object);
	}

	EntityInstance* ResourceManagerService::findEntity(const std::string& inID) const
	{
		EntityByIDMap::const_iterator pos = mEntities.find(inID);
		if (pos == mEntities.end())
			return nullptr;

		return pos->second.get();
	}

}
