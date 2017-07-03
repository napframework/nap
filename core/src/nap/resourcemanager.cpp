#include "resourcemanager.h"
#include "rtti/rttiutilities.h"
#include "directorywatcher.h"
#include "rtti/jsonreader.h"
#include "rtti/factory.h"
#include "nap/core.h"
#include "objectptr.h"
#include "entityinstance.h"
#include "componentinstance.h"

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


	class RTTIObjectGraphItem
	{
	public:
		using Type = rtti::RTTIObject*;

		static const RTTIObjectGraphItem create(Type object)
		{
			RTTIObjectGraphItem item;
			item.mType = EType::Object;
			item.mObject = object;
			
			return item;
		}

		const std::string getID() const
		{
			assert(mType == EType::File || mType == EType::Object);

			if (mType == EType::File)
				return mFilename;
			else 
				return mObject->mID;
		}

		uint8_t getType() const { return (uint8_t)mType; }

		bool getPointers(std::vector<RTTIObjectGraphItem>& pointers, utility::ErrorState& errorState) const
		{
			std::vector<rtti::ObjectLink> object_links;
			rtti::findObjectLinks(*mObject, object_links);

			for (const rtti::ObjectLink& link : object_links)
			{
				RTTIObjectGraphItem item;
				item.mType = EType::Object;
				item.mObject = link.mTarget;
				pointers.push_back(item);
			}

			std::vector<std::string> file_links;
			rtti::findFileLinks(*mObject, file_links);

			for (std::string& filename : file_links)
			{
				RTTIObjectGraphItem item;
				item.mType = EType::File;
				item.mFilename = filename;
				pointers.push_back(item);
			}
			
			return true;
		}

		enum class EType : uint8_t
		{
			Object,
			File
		};

		EType				mType;
		std::string			mFilename;
		rtti::RTTIObject*	mObject = nullptr;
	};

	class TypeInfoGraphItem
	{
	public:
		using Type = ObjectPtr<ComponentResource>;
		using ComponentMap = std::unordered_map<rtti::TypeInfo, std::vector<Type>>;

		static const TypeInfoGraphItem create(const ComponentMap& componentMap, Type inComponent)
		{
			TypeInfoGraphItem item;
			item.mComponentMap = &componentMap;
			item.mComponent = inComponent;

			return item;
		}

		const std::string getID() const
		{
			return mComponent->mID;
		}

		bool getPointers(std::vector<TypeInfoGraphItem>& pointers, utility::ErrorState& errorState) const
		{
			std::vector<rtti::TypeInfo> dependent_types;
			mComponent->getDependentComponents(dependent_types);

			for (rtti::TypeInfo& type : dependent_types)
			{
				ComponentMap::const_iterator dependent_component = mComponentMap->find(type);
				if (!errorState.check(dependent_component != mComponentMap->end(), "Component %s was unable to find dependent component of type %s", getID().c_str(), type.get_name().data()))
					return false;

				const std::vector<ObjectPtr<ComponentResource>> components = dependent_component->second;
				for (Type component : components)
				{
					TypeInfoGraphItem item;
					item.mComponent = component;
					item.mComponentMap = mComponentMap;
					pointers.push_back(item);
				}
			}

			return true;
		}

		const ComponentMap*	mComponentMap = nullptr;
		Type				mComponent = nullptr;
	};

	/** 
	* Scans objects for links to other objects and other files. This is done through RTTI scanning, where
	* link properties are examined. The end result is a list of Nodes and Edges that can be used to traverse
	* the graph.
	*/
	template<typename ITEM>
	class ObjectGraph final
	{
	public:
		struct Node;

		using ItemList = std::vector<typename ITEM::Type>;

		/**
		* Represent an 'edge' in the ObjectGraph. This is a link from an object to either a file or another object.
		*/
		struct Edge
		{
			Node*		mSource = nullptr;		// Link source
			Node*		mDest = nullptr;		// Link target
		};


		/*
		* Represents a node in the ObjectGraph, which can be an object or a file.
		*/
		struct Node
		{
			int					mDepth = -1;			// Depth of the node is calculated during build, it represents at what level from the root this node is.
			ITEM				mItem;					// 
			std::vector<Edge*>	mIncomingEdges;			// List of incoming edges
			std::vector<Edge*>	mOutgoingEdges;			// List of outgoing edges
		};


		/*
		* Builds the object graph. If building fails, return false. 
		* @param objectList : list of objects to build the graph from.
		* @param errorState: if false is returned, contains error information.
		*/
		bool build(const ItemList& objectList, std::function<ITEM(typename ITEM::Type)> creationFunction, utility::ErrorState& errorState)
		{
			using ObjectMap = std::map<std::string, ITEM>;
			ObjectMap object_map;

			// Build map from ID => object
			for (typename ITEM::Type object : objectList)
			{
				object_map.insert({ object->mID, creationFunction(object) });
			}

			// Scan all objects for rtti links and build data structure
			for (typename ITEM::Type object : objectList)
			{
				Node* source_node = getOrCreateItemNode(creationFunction(object));

				std::vector<ITEM> pointees;
				if (!source_node->mItem.getPointers(pointees, errorState))
					return false;

				for (ITEM& item : pointees)
				{
					Edge* edge = new Edge();
					edge->mSource = source_node;
					edge->mDest = getOrCreateItemNode(item);;
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
		Node* findNode(const std::string& ID)
		{
			NodeMap::iterator iter = mNodes.find(ID);
			if (iter == mNodes.end())
				return nullptr;
			
			return iter->second.get();
		}

		std::vector<Node*> getSortedNodes()
		{
			std::vector<Node*> sorted_nodes;
			sorted_nodes.reserve(mNodes.size());
			for (auto& kvp : mNodes)
				sorted_nodes.push_back(kvp.second.get());

			std::sort(sorted_nodes.begin(), sorted_nodes.end(), [](Node* nodeA, Node* nodeB) { return nodeA->mDepth > nodeB->mDepth; });

			return sorted_nodes;
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


		Node* getOrCreateItemNode(const ITEM& item)
		{
			Node* result = nullptr;
			NodeMap::iterator iter = mNodes.find(item.getID());
			if (iter == mNodes.end())
			{
				auto node = std::make_unique<Node>();
				node->mItem = item;
				result = node.get();
				mNodes.insert(std::make_pair(item.getID(), std::move(node)));
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

	using RTTIObjectGraph = ObjectGraph<RTTIObjectGraphItem>;
	using TypeDependencyGraph = ObjectGraph<TypeInfoGraphItem>;


	/**
	* Walks object graph by traversing incoming edges and pushing the results in an array.
	* @param node: start node to traverse incoming edges from.
	* @param incomingObjects: output of the function.
	*/
	template<typename GRAPHTYPE>
	static void addIncomingObjectsRecursive(typename GRAPHTYPE::Node* node, std::set<typename GRAPHTYPE::Node*>& incomingObjects)
	{
		if (incomingObjects.find(node) != incomingObjects.end())
			return;

		incomingObjects.insert(node);

		for (typename GRAPHTYPE::Edge* incoming_edge : node->mIncomingEdges)
			addIncomingObjectsRecursive<GRAPHTYPE>(incoming_edge->mSource, incomingObjects);
	}


	/**
	 * Helper function to generate a unique ID for an entity or component instance, based on instances already created
	 */
	static const std::string generateInstanceID(const std::string& baseID, EntityCreationParameters& entityCreationParams)
	{
		std::string result = baseID;

		int index = 0;
		while (entityCreationParams.mAllInstancesByID.find(result) != entityCreationParams.mAllInstancesByID.end())
			result = utility::stringFormat("%s_%d", baseID.c_str(), index++);

		return result;
	}


	const std::string getInstanceID(const std::string& baseID)
	{
		return baseID + "_instance";
	}


	//////////////////////////////////////////////////////////////////////////
	// ResourceManagerService
	//////////////////////////////////////////////////////////////////////////


	ResourceManagerService::ResourceManagerService() :
		mDirectoryWatcher(std::make_unique<DirectoryWatcher>())
	{ 
	}


	void ResourceManagerService::initialized()
	{
		mRootEntity = std::make_unique<EntityInstance>(getCore());
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

		RTTIObjectGraph object_graph;
		if (!object_graph.build(all_objects, [](rtti::RTTIObject* object) { return RTTIObjectGraphItem::create(object); }, errorState))
			return false;		

		// Traverse graph for incoming links and add all of them
		std::set<RTTIObjectGraph::Node*> objects_to_init;
		for (const std::string& dirty_node : dirty_nodes)
		{
			RTTIObjectGraph::Node* node = object_graph.findNode(dirty_node);

			// In the case that file links change as part of the file modification(s), it's possible for the dirty node to not be present in the ObjectGraph,
			// so we can't assert here but need to deal with that case.
			if (node != nullptr)
				addIncomingObjectsRecursive<RTTIObjectGraph>(node, objects_to_init);
		}

		// Sort on graph depth for the correct init() order
		std::vector<RTTIObjectGraph::Node*> sorted_objects_to_init;
		for (RTTIObjectGraph::Node* object_to_init : objects_to_init)
			sorted_objects_to_init.push_back(object_to_init);

		std::sort(sorted_objects_to_init.begin(), sorted_objects_to_init.end(),
			[](RTTIObjectGraph::Node* nodeA, RTTIObjectGraph::Node* nodeB) { return nodeA->mDepth > nodeB->mDepth; });

		for (RTTIObjectGraph::Node* sorted_object_to_init : sorted_objects_to_init)
			if (sorted_object_to_init->mItem.mObject != nullptr)
				objectsToInit.push_back(sorted_object_to_init->mItem.mObject->mID);

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

	const ObjectPtr<EntityInstance> ResourceManagerService::createEntity(const EntityResource& entityResource, EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState)
	{
		// Create a single entity
		std::vector<std::string> generated_ids;
		std::vector<const EntityResource*> entityResources;
		entityResources.push_back(&entityResource);
		bool result = createEntities(entityResources, entityCreationParams, generated_ids, errorState);
		if (!result)
			return false;

		assert(generated_ids.size() == 1);
		return entityCreationParams.mEntitiesByID.find(generated_ids[0])->second.get();
	}

	void addComponentsByType(std::unordered_map<rtti::TypeInfo, std::vector<ObjectPtr<ComponentResource>>>& componentsByType, const ObjectPtr<ComponentResource>& component, const rtti::TypeInfo& type)
	{
		componentsByType[type].push_back(component);
		for (const rtti::TypeInfo& base_type : type.get_base_classes())
			addComponentsByType(componentsByType, component, base_type);
	}

	bool ResourceManagerService::createEntities(const std::vector<const EntityResource*>& entityResources, EntityCreationParameters& entityCreationParams, std::vector<std::string>& generatedEntityIDs, utility::ErrorState& errorState)
	{
		std::unordered_map<ComponentResource*, ComponentInstance*> new_component_instances;

		// Create all entity instances and component instances
		for (const EntityResource* entity_resource : entityResources)
		{
			EntityInstance* entity_instance = new EntityInstance(getCore());
			entity_instance->mID = generateInstanceID(getInstanceID(entity_resource->mID), entityCreationParams);

			entityCreationParams.mEntitiesByID.emplace(std::make_pair(entity_instance->mID, std::move(std::unique_ptr<EntityInstance>(entity_instance))));
			entityCreationParams.mAllInstancesByID.insert(std::make_pair(entity_instance->mID, entity_instance));
			generatedEntityIDs.push_back(entity_instance->mID);

			for (auto& component_resource : entity_resource->mComponents)
			{
				const rtti::TypeInfo& instance_type = component_resource->getInstanceType();
				assert(instance_type.can_create_instance());

				std::unique_ptr<ComponentInstance> component_instance(instance_type.create<ComponentInstance>({ *entity_instance }));
				assert(component_instance);
				component_instance->mID = generateInstanceID(getInstanceID(component_resource->mID), entityCreationParams);

				new_component_instances.insert(std::make_pair(component_resource.get(), component_instance.get()));
				entityCreationParams.mAllInstancesByID.insert(std::make_pair(component_instance->mID, component_instance.get()));
				entity_instance->addComponent(std::move(component_instance));
			}
		}

		// Now that all entities are created, make sure that parent-child relations are set correctly
		for (const EntityResource* entity_resource : entityResources)
		{
			EntityByIDMap::iterator entity_instance = entityCreationParams.mEntitiesByID.find(getInstanceID(entity_resource->mID));
			assert(entity_instance != entityCreationParams.mEntitiesByID.end());

			for (const ObjectPtr<EntityResource>& child_entity_resource : entity_resource->mChildren)
			{
				EntityByIDMap::iterator child_entity_instance = entityCreationParams.mEntitiesByID.find(getInstanceID(child_entity_resource->mID));
				assert(child_entity_instance != entityCreationParams.mEntitiesByID.end());
				entity_instance->second->addChild(*child_entity_instance->second);
			}
		}

		// Now that all entities are setup correctly, initialize the component instances with the
		// component resource data.
		for (const EntityResource* entity_resource : entityResources)
		{
			std::unordered_map<rtti::TypeInfo, std::vector<ObjectPtr<ComponentResource>>> components_by_type;
			for (auto& node : entity_resource->mComponents)
				addComponentsByType(components_by_type, node.get(), node->get_type());

			TypeDependencyGraph graph;
			if (!graph.build(entity_resource->mComponents, [&components_by_type](ObjectPtr<ComponentResource>& component) { return TypeInfoGraphItem::create(components_by_type, component); }, errorState))
				return false;

			std::vector<TypeDependencyGraph::Node*> sorted_nodes = graph.getSortedNodes();

			for (TypeDependencyGraph::Node* node : sorted_nodes)
			{
				auto& pos = new_component_instances.find(node->mItem.mComponent.get());
				assert(pos != new_component_instances.end());

				if (!pos->second->init(node->mItem.mComponent, entityCreationParams, errorState))
					return false;
			}
		}

		return true;
	}


	bool ResourceManagerService::initEntities(ObjectByIDMap& objectsToUpdate, utility::ErrorState& errorState)
	{
		// Build list of all entities we need to update. We need to use the objects in objectsToUpdate over those already in the ResourceManager
		// In essence, objectsToUpdate functions as an 'overlay' on top of the ResourceManager
		std::vector<const EntityResource*> entities_to_spawn;
		
		// First add all EntityResources in the list of objects to update
		for (auto& kvp : objectsToUpdate)
		{
			if (kvp.second->get_type() != RTTI_OF(EntityResource))
				continue;

			EntityResource* resource = rtti_cast<EntityResource>(kvp.second.get());
			if (resource->mAutoSpawn)
				entities_to_spawn.push_back(resource);
		}

		// Next, go through all EntityResources currently in the resource manager and add them if they're not in the list of objects to update
		for (auto& kvp : mObjects)
		{
			if (kvp.second->get_type() != RTTI_OF(EntityResource))
				continue;

			ObjectByIDMap::const_iterator object_to_update = objectsToUpdate.find(kvp.first);
			if (object_to_update == objectsToUpdate.end())
			{
				EntityResource* resource = rtti_cast<EntityResource>(kvp.second.get());
				if (resource->mAutoSpawn)
					entities_to_spawn.push_back(resource);
			}
		}

		std::vector<std::string> generated_ids;
		EntityCreationParameters entityCreationParams;
		if (!createEntities(entities_to_spawn, entityCreationParams, generated_ids, errorState))
		{
			return false;
		}

		// Start with an empty root and add all entities without a parent to the root
		mRootEntity->clearChildren();
		for (auto& kvp : entityCreationParams.mEntitiesByID)
		{
			if (kvp.second->getParent() == nullptr)
				mRootEntity->addChild(*kvp.second);
		}

		patchObjectPtrs(entityCreationParams.mAllInstancesByID);

		// Replace entities currently in the resource manager with the new set
		mEntities = std::move(entityCreationParams.mEntitiesByID);
		
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


	const ObjectPtr<EntityInstance> ResourceManagerService::findEntity(const std::string& inID) const
	{
		EntityByIDMap::const_iterator pos = mEntities.find(getInstanceID(inID));
		if (pos == mEntities.end())
			return nullptr;

		return pos->second.get();
	}

}
