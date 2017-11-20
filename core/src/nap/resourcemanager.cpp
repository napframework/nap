#include "resourcemanager.h"
#include "objectgraph.h"
#include "entityptr.h"
#include "componentptr.h"
#include "logger.h"
#include "core.h"
#include <utility/fileutils.h>
#include <utility/stringutils.h>
#include <rtti/rttiutilities.h>
#include <rtti/jsonreader.h>
#include <rtti/pythonmodule.h>
#include <nap/core.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ResourceManager)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_FUNCTION("findEntity", &nap::ResourceManager::findEntity)
	RTTI_FUNCTION("findObject", (const nap::ObjectPtr<nap::rtti::RTTIObject> (nap::ResourceManager::*)(const std::string&))&nap::ResourceManager::findObject)
RTTI_END_CLASS

namespace nap
{
	using namespace rtti;


	//////////////////////////////////////////////////////////////////////////
	// ResourceManager::RollbackHelper
	//////////////////////////////////////////////////////////////////////////


	ResourceManager::RollbackHelper::RollbackHelper(ResourceManager& service) :
		mService(service)
	{
	}


	ResourceManager::RollbackHelper::~RollbackHelper()
	{
		if (mPatchObjects)
			mService.patchObjectPtrs(mService.mObjects);
	}


	void ResourceManager::RollbackHelper::clear()
	{
		mPatchObjects = false;
	}


	//////////////////////////////////////////////////////////////////////////
	// RTTIObjectGraphItem
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Item class for ObjectGraph usage.
	 * Wraps both an RTTIObject and a File object (by filename).
	 * Uses RTTI traversal to scan pointers to other objects and pointers to files.
	 */
	class RTTIObjectGraphItem
	{
	public:
		using Type = rtti::RTTIObject*;
		using ObjectsByTypeMap = std::unordered_map<rtti::TypeInfo, std::vector<RTTIObject*>>;

		enum class EType : uint8_t
		{
			Object,
			File
		};

		/**
		 * Creates a graph item.
		 * @param object Object to wrap in the item that is created.
		 */
		static const RTTIObjectGraphItem create(rtti::RTTIObject* object, const ObjectsByTypeMap& objectsByType)
		{
			RTTIObjectGraphItem item;
			item.mType = EType::Object;
			item.mObject = object;
			item.mObjectsByType = &objectsByType;
			
			return item;
		}

		/**
		 * @return ID of the item. For objects, the ID is the object ID, for files, it is the filename.
		 */
		const std::string getID() const
		{
			assert(mType == EType::File || mType == EType::Object);

			if (mType == EType::File)
				return mFilename;
			else 
				return mObject->mID;
		}

		/**
		 * @return EType of the type (file or object).
		 */
		uint8_t getType() const { return (uint8_t)mType; }


		/**
		 * Performs rtti traversal of pointers to both files and objects.
		 * @param pointees Output parameter, contains all objects and files this object points to.
		 * @param errorState If false is returned, contains information about the error.
		 * @return true is succeeded, false otherwise.
		 */
		bool getPointees(std::vector<RTTIObjectGraphItem>& pointees, utility::ErrorState& errorState) const
		{
			Component* component = rtti_cast<Component>(mObject);
			if (component != nullptr)
			{
				std::vector<rtti::TypeInfo> dependent_types;
				component->getDependentComponents(dependent_types);

				for (rtti::TypeInfo& type : dependent_types)
				{
					ObjectsByTypeMap::const_iterator dependent_component = mObjectsByType->find(type);
					if (!errorState.check(dependent_component != mObjectsByType->end(), "Component %s was unable to find dependent component of type %s", getID().c_str(), type.get_name().data()))
						return false;

					const std::vector<RTTIObject*> components = dependent_component->second;
					for (RTTIObject* component : components)
					{
						RTTIObjectGraphItem item;
						item.mType = EType::Object;
						item.mObject = component;
						item.mObjectsByType = mObjectsByType;
						pointees.push_back(item);
					}
				}
			}

			std::vector<rtti::ObjectLink> object_links;
			rtti::findObjectLinks(*mObject, object_links);

			for (const rtti::ObjectLink& link : object_links)
			{
				if (link.mTarget == nullptr)
					continue;

				RTTIObjectGraphItem item;
				item.mType = EType::Object;
				item.mObject = link.mTarget;
				item.mObjectsByType = mObjectsByType;
				pointees.push_back(item);
			}

			std::vector<std::string> file_links;
			rtti::findFileLinks(*mObject, file_links);

			for (std::string& filename : file_links)
			{
				RTTIObjectGraphItem item;
				item.mType = EType::File;
				item.mFilename = filename;
				item.mObjectsByType = mObjectsByType;
				pointees.push_back(item);
			}
			
			return true;
		}
		
		EType				mType;					// Type: file or object
		std::string			mFilename;				// If type is file, contains filename
		rtti::RTTIObject*	mObject = nullptr;		// If type is object, contains object pointer
		const ObjectsByTypeMap*		mObjectsByType = nullptr;	// All objects sorted by type
	};


	using RTTIObjectGraph = ObjectGraph<RTTIObjectGraphItem>;

	//////////////////////////////////////////////////////////////////////////

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


	/** 
	 * Recursively adds all types to the componentsByType map. Notice that all base classes are inserted into the map as well to make sure we can perform 
	 * is_derived_from check against this map.
	 */
	void addComponentsByType(std::unordered_map<rtti::TypeInfo, std::vector<Component*>>& componentsByType, Component* component, const rtti::TypeInfo& type)
	{
		componentsByType[type].push_back(component);
		for (const rtti::TypeInfo& base_type : type.get_base_classes())
			addComponentsByType(componentsByType, component, base_type);
	}


	/**
	 * Performs a graph traversal of all objects in the graph that have the ID that matches any of the objects in @param dirtyObjects.
     * All the edges in the graph are traversed in the incoming direction. Any object that is encountered is added to the set.
     * Finally, all objects that were visited are sorted on graph depth.
 	 */
	void traverseAndSortIncomingObjects(const std::unordered_map<std::string, rtti::RTTIObject*>& dirtyObjects, const RTTIObjectGraph& objectGraph, std::vector<std::string>& sortedObjects)
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
		mRootEntity = std::make_unique<EntityInstance>(mCore, nullptr);
	}


	void ResourceManager::update(double elapsedTime)
	{
		getRootEntity().update(elapsedTime);
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

		// Build map of objects per type, this is used for tracking type dependencies while building the graph
		RTTIObjectGraphItem::ObjectsByTypeMap objects_by_type;
		for (rtti::RTTIObject* object : all_objects)
			objects_by_type[object->get_type()].emplace_back(object);

		if (!objectGraph.build(all_objects, [&objects_by_type](rtti::RTTIObject* object) { return RTTIObjectGraphItem::create(object, objects_by_type); }, errorState))
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
		std::unordered_map<std::string, nap::RTTIObject*> dirty_nodes;
		for (auto& kvp : objectsToUpdate)
			dirty_nodes.insert(std::make_pair(kvp.first, kvp.second.get()));

		// Add externally changed file that caused load of this json file
		if (!externalChangedFile.empty())
			dirty_nodes.insert(std::make_pair(externalChangedFile, nullptr));

		// Traverse graph for incoming links and add all of them, and sort them based on graph depth
		traverseAndSortIncomingObjects(dirty_nodes, objectGraph, objectsToInit);
	}


	bool ResourceManager::loadFile(const std::string& filename, utility::ErrorState& errorState)
	{
		return loadFile(filename, std::string(), errorState);
	}


	// Resolves all the pointers in @unresolvedPointers against both the ResourceManager's objects and the @objectsToUpdate map. The @objectsToUpdate array
	// functions as an overlay on top of the ResourceManager.
	// Custom pointers are supported through exposed RTTI functions on pointer objects: a static function 'translateTargetID' will convert any ID to a target ID
	// and the member function 'assign' will assign the pointer value to the pointer object.
	bool ResourceManager::resolvePointers(ObjectByIDMap& objectsToUpdate, const UnresolvedPointerList& unresolvedPointers, utility::ErrorState& errorState)
	{
		for (const UnresolvedPointer& unresolved_pointer : unresolvedPointers)
		{
			rtti::ResolvedRTTIPath resolved_path;
			if (!errorState.check(unresolved_pointer.mRTTIPath.resolve(unresolved_pointer.mObject, resolved_path), "Failed to resolve RTTIPath %s", unresolved_pointer.mRTTIPath.toString().c_str()))
				return false;

			std::string target_id = unresolved_pointer.mTargetID;			

			// If the type that we're processing has a function to translate the ID read from json into a different ID, we call it and use that ID.
			// This is used for pointers that have a different format in json.
			rttr::method translate_string_method = rtti::findMethodRecursive(resolved_path.getType(), "translateTargetID");
			if (translate_string_method.is_valid())
			{
				rttr::variant translate_result = translate_string_method.invoke(rttr::instance(), target_id);
				target_id = translate_result.to_string();
			}

			// Objects in objectsToUpdate have preference over the manager's objects. 
			RTTIObject* target_object = nullptr;
            auto object_to_update = objectsToUpdate.find(target_id);
			if (object_to_update == objectsToUpdate.end())
				target_object = findObject(target_id).get();
			else
				target_object = object_to_update->second.get();

			if (!errorState.check(target_object != nullptr, "Unable to resolve link to object %s from attribute %s", target_id.c_str(), unresolved_pointer.mRTTIPath.toString().c_str()))
				return false;

			rtti::TypeInfo resolved_path_type = resolved_path.getType();
			rtti::TypeInfo actual_type = resolved_path_type.is_wrapper() ? resolved_path_type.get_wrapped_type() : resolved_path_type;

			if (!errorState.check(target_object->get_type().is_derived_from(actual_type), "Failed to resolve pointer: target of pointer {%s}:%s is of the wrong type (found '%s', expected '%s')",
				unresolved_pointer.mObject->mID.c_str(), unresolved_pointer.mRTTIPath.toString().c_str(), target_object->get_type().get_name().data(), actual_type.get_raw_type().get_name().data()))
			{
				return false;
			}

			assert(actual_type.is_pointer());

			// If the type that we're processing has a function to assign the pointer value, we use it.
			rtti::Variant target_value = target_object;
			rttr::method assign_method = rtti::findMethodRecursive(resolved_path.getType(), "assign");
			if (assign_method.is_valid())
			{
				target_value = resolved_path.getValue();
				assign_method.invoke(target_value, unresolved_pointer.mTargetID, *target_object);
			}

			bool succeeded = resolved_path.setValue(target_value);
			if (!errorState.check(succeeded, "Failed to resolve pointer for: %s", target_object->mID.c_str()))
				return false;
		}

		return true;
	}


	// inits all objects 
	bool ResourceManager::initObjects(const std::vector<std::string>& objectsToInit, const ObjectByIDMap& objectsToUpdate, utility::ErrorState& errorState)
	{
        // Init all objects in the correct order
        for (const std::string& id : objectsToInit)
        {
	        rtti::RTTIObject* object = nullptr;
        
	        // We perform lookup by ID. Objects in objectsToUpdate have preference over the manager's objects.
	        ObjectByIDMap::const_iterator updated_object = objectsToUpdate.find(id);
	        if (updated_object != objectsToUpdate.end())
		        object = updated_object->second.get();
	        else
		        object = findObject(id).get();

			if (!object->init(errorState)) {
				Logger::warn("Couldn't initialise object '%s'", id.c_str());
		        return false;
			}
        }

        return true;
	}


	const ObjectPtr<EntityInstance> ResourceManager::createEntity(const nap::Entity& Entity, EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState)
	{
		// Create a single entity
		std::vector<std::string> generated_ids;
		std::vector<const nap::Entity*> entityResources;
		entityResources.push_back(&Entity);
		bool result = createEntities(entityResources, entityCreationParams, generated_ids, errorState);
		if (!result)
			return nullptr;

		assert(generated_ids.size() == 1);
		return entityCreationParams.mEntityInstancesByID.find(generated_ids[0])->second.get();
	}


	ComponentInstance* ResourceManager::sResolveComponentInstancePath(ComponentInstance* sourceComponentInstance, const std::string& targetComponentInstancePath, Component* targetComponentResource,
		const ResourceManager::RootEntityInstanceMap& rootEntityInstances, const EntityCreationParameters::ComponentInstanceMap& componentInstances, utility::ErrorState& errorState)
	{
		ComponentInstance* target_component_instance = nullptr;

		// Split the path into its components
		std::vector<std::string> path_components;
		utility::splitString(targetComponentInstancePath, '/', path_components);

		// If the path consists out of a single element, we're linking directly to a specific component so we can just use that
		if (path_components.size()  == 1)
		{
			EntityCreationParameters::ComponentInstanceMap::const_iterator pos = componentInstances.find(targetComponentResource);
			if (pos != componentInstances.end())
			{
				// If we're linking directly to a specific component, ensure there is no ambiguity
				if (!errorState.check(pos->second.size() == 1, "Encountered ambiguous component pointer"))
					return nullptr;

				target_component_instance = pos->second[0];
			}
		}
		else
		{
			// The path consists out of multiple elements, indicating either a relative or absolute path to a component instance.
			// We need to determine the entity that the path 'starts' at so that we can resolve the rest
			nap::EntityInstance* current_entity = nullptr;			
			const std::string& root_element = path_components[0];

			// If the part starts with a period, it means we should start in the entity that the source component is in
			if (root_element == ".")
			{
				current_entity = sourceComponentInstance->getEntityInstance();
			}
			else if (root_element == "..")
			{
				// Part starts with a double period; start at the parent of the entity that the source component is in
				current_entity = sourceComponentInstance->getEntityInstance()->getParent();
				if (!errorState.check(current_entity != nullptr, "Error resolving ComponentPtr with path %s: path starts with '..' but source entity has no parent", targetComponentInstancePath.c_str()))
					return nullptr;
			}
			else
			{
				// No relative path components: the first element on the path represents the ID of a root entity. We find it here.
				RootEntityInstanceMap::const_iterator pos = rootEntityInstances.find(root_element);
				if (!errorState.check(pos != rootEntityInstances.end(), "Error resolving ComponentPtr with path %s: root entity '%s' not found", targetComponentInstancePath.c_str(), root_element.c_str()))
					return nullptr;

				current_entity = pos->second;
			}

			// Now resolve the rest of the path. Note that we iterate from the second element (because we've already processed the root) to the second-to-last element (because the last element specifies the component we're looking for )
			for (int index = 1; index < path_components.size() - 1; ++index)
			{
				const std::string& part = path_components[index];
				
				// If we encounter a double period, go up another level
				if (part == "..")
				{
					current_entity = current_entity->getParent();
					if (!errorState.check(current_entity != nullptr, "Error resolving ComponentPtr with path %s: path contains a '..' at a point where there are no more parents", targetComponentInstancePath.c_str()))
						return nullptr;
				}
				else if (part != ".")
				{
					// If we encountered a non-relative component, we need to look for a child entity of the current entity that matches the child specifier

					// Split the child specifier on ':'. Note that the ':' is optional and is only used to disambiguate between multiple children
					std::vector<std::string> element_parts;
					utility::splitString(part, ':', element_parts);
					if (!errorState.check(element_parts.size() <= 2, "Error resolving ComponentPtr with path %s: path contains a child specifier with an invalid format (multiple colons found)", targetComponentInstancePath.c_str()))
						return nullptr;

					// Find all child entities matching the ID
					std::vector<EntityInstance*> matching_children;
					for (EntityInstance* entity_instance : current_entity->getChildren())
						if (entity_instance->getEntity()->mID == element_parts[0])
							matching_children.push_back(entity_instance);

					// There must be at least one match
					if (!errorState.check(matching_children.size() != 0, "Error resolving ComponentPtr with path %s: child with ID '%s' not found in entity with ID '%s'", targetComponentInstancePath.c_str(), element_parts[0].c_str(), current_entity->getEntity()->mID.c_str()))
						return nullptr;

					// If the child specifier was a single ID, there must be only a single match and we set that entity as the new current entity
					if (element_parts.size() == 1)
					{
						if (!errorState.check(matching_children.size() == 1, "Error resolving ComponentPtr with path %s: path is ambiguous; found %d children with ID '%s' in entity with ID '%s'. Use the child specifier syntax 'child_id:child_index' to disambiguate.", targetComponentInstancePath.c_str(), matching_children.size(), element_parts[0].c_str(), current_entity->getEntity()->mID.c_str()))
							return nullptr;

						current_entity = matching_children[0];
					}
					else
					{
						// The child specifier contained an index to disambiguate between multiple children with the same ID; parse the index
						int array_index;
						if (!errorState.check(sscanf(element_parts[1].c_str(), "%d", &array_index) == 1, "Error resolving ComponentPtr with path %s: path contains a child specifier with an invalid format (unable to parse int from %s)", targetComponentInstancePath.c_str(), element_parts[1].c_str()))
							return nullptr;

						if (!errorState.check(array_index < matching_children.size(), "Error resolving ComponentPtr with path %s: path contains an invalid child specifier; found %d eligible children but index %d is out of range", targetComponentInstancePath.c_str(), matching_children.size(), array_index))
							return nullptr;

						// Use the child with the specified index as current entity
						current_entity = matching_children[array_index];
					}
				}
			}

			// Now that we've gone through the path, we know the current entity must contain a component with an ID equal to the last element on the path. We look for it here.
			assert(current_entity != nullptr);
			for (ComponentInstance* component : current_entity->getComponents())
			{
				if (component->getComponent()->mID == path_components.back())
				{
					target_component_instance = component;
					break;
				}
			}
		}

		return target_component_instance;
	}


	/**
	 * This function resolves pointers in a ComponentResource of the types EntityPtr and ComponentInstancePtr. Although the RTTI resource pointers in EntityPtr
	 * and ComponentInstancePtr have already been resolved by the regular RTTI pointer resolving step, this step is meant explicitly to resolve pointers
	 * to instances that are stored internally in the ComponentInstancePtr and EntityPtr.
	 * The resolving step of entities and components is more difficult than regular objects, as the entity/component structure is mirrored into
	 * a resource structure (the static objects from json) and instances (the runtime counterpart of the resources). EntityPtr and ComponentInstancePtr
	 * are pointers that live on the resource object as the resources need to specify what other resource they are pointing to. However, the
	 * instantiated object often needs to point to other instantiated objects. In this function, we fill in the instance pointers in EntityPtr and 
	 * ComponentInstancePtr, so that the instance can get to the instance pointer through it's resource.
	 */
	bool ResourceManager::sResolveComponentPointers(EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState)
	{
		RootEntityInstanceMap root_entity_instances;
		for (auto& kvp : entityCreationParams.mEntityInstancesByID)
		{
			EntityInstance* entity_instance = kvp.second.get();
			if (entity_instance->getParent() == nullptr)
				root_entity_instances.emplace(std::make_pair(kvp.first, entity_instance));
		}

		// We go over all component instances and resolve all Entity & Component pointers
		for (auto& kvp : entityCreationParams.mComponentInstanceMap)
		{
			Component* source_component_resource = kvp.first;

			// Resolve the component pointers for all instances of this component resource
			for (ComponentInstance* source_component_instance : kvp.second)
			{
				// Resolve all links for this instance
				ComponentInstance::LinkMap& linkmap = source_component_instance->mLinkMap;
				for (auto& link : linkmap)
				{
					nap::Component* target_component_resource = link.first;					

					// It's possible for the same ComponentInstance to link to a particular component multiple times, so we need to resolve all those links individually (the paths might be different)
					for (ComponentInstance::TargetComponentLink& target_component_link : link.second)
					{
						// Resolve the path to the target ComponentInstance
						nap::ComponentInstance* target_component_instance = sResolveComponentInstancePath(source_component_instance, target_component_link.mInstancePath, target_component_resource, root_entity_instances, entityCreationParams.mComponentInstanceMap, errorState);
						if (!errorState.check(target_component_instance != nullptr, "Invalid component pointer"))
							return false;

						// Update the ComponentInstancePtr
						*target_component_link.mTargetPtr = target_component_instance;
					}
				}
			}

			// Iterate over all the pointers in the component resource. Note that findObjectLinks returns *all* types of pointers on the object, 
			// but we're only interested in EntityPtrs since other pointers will have been resolved during the load.
			std::vector<rtti::ObjectLink> links;
			rtti::findObjectLinks(*source_component_resource, links);

			for (rtti::ObjectLink& link : links)
			{
				rtti::ResolvedRTTIPath resolved_path;
				if (!errorState.check(link.mSourcePath.resolve(source_component_resource, resolved_path), "Encountered link from object %s that could not be resolved: %s", source_component_resource->mID.c_str(), link.mSourcePath.toString().c_str()))
					return false;

				// Resolve EntityPtr
				if (resolved_path.getType() == RTTI_OF(EntityPtr))
				{
					EntityPtr entity_ptr = resolved_path.getValue().convert<EntityPtr>();
					nap::Entity* target_entity_resource = entity_ptr.getResource();

					// Skip null targets
					if (target_entity_resource == nullptr)
						continue;

					// Only AutoSpawn resources have a one-to-one relationship between resource and instance. We do not support pointers to non-AutoSpawn objects
					if (!errorState.check(target_entity_resource->mAutoSpawn, "Encountered pointer from {%s}:%s to non-AutoSpawn entity %s. This is not supported.", source_component_resource->mID.c_str(), link.mSourcePath.toString().c_str(), target_entity_resource->mID.c_str()))
						return false;

					// Find the EntityInstance and fill it in in the EntityPtr.mInstance
					EntityByIDMap::iterator target_entity_instance = entityCreationParams.mEntityInstancesByID.find(getInstanceID(target_entity_resource->mID));
					assert(target_entity_instance != entityCreationParams.mEntityInstancesByID.end());
					entity_ptr.mInstance = target_entity_instance->second.get();

					resolved_path.setValue(entity_ptr);
				}
			}
		}

		return true;
	}


	static bool sInitComponents(EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState)
	{
		std::vector<RTTIObjectGraph::Node*> sorted_nodes = entityCreationParams.mObjectGraph->getSortedNodes();

		for (RTTIObjectGraph::Node* node : sorted_nodes)
		{
			Component* component = rtti_cast<Component>(node->mItem.mObject);
			if (component == nullptr)
				continue;

			auto pos = entityCreationParams.mComponentInstanceMap.find(component);

			// It's possible for there to be objects in the graph that were not created during this createEntities call,
			// in the case where a component has ComponentPtrs to components of other entities that have been spawned in a previous iteration
			if (pos == entityCreationParams.mComponentInstanceMap.end())
				continue;

			for (ComponentInstance* component_instance : pos->second)
				if (!component_instance->init(errorState))
				return false;
		}

		return true;
	}


	bool ResourceManager::createEntities(const std::vector<const nap::Entity*>& entityResources, EntityCreationParameters& entityCreationParams, std::vector<std::string>& generatedEntityIDs, utility::ErrorState& errorState)
	{
		std::unordered_set<const Entity*> rootEntityResources;
		rootEntityResources.insert(entityResources.begin(), entityResources.end());

		for (const nap::Entity* entity_resource : entityResources)
		{
			for (auto& child : entity_resource->mChildren)
				rootEntityResources.erase(child.get());
		}

		// Create all entity instances and component instances
		for (const nap::Entity* entity_resource : rootEntityResources)
		{
			EntityInstance* entity_instance = new EntityInstance(mCore, entity_resource);
			entity_instance->mID = generateInstanceID(getInstanceID(entity_resource->mID), entityCreationParams);

			entityCreationParams.mEntityInstancesByID.emplace(std::make_pair(entity_instance->mID, std::unique_ptr<EntityInstance>(entity_instance)));
			entityCreationParams.mAllInstancesByID.insert(std::make_pair(entity_instance->mID, entity_instance));
			generatedEntityIDs.push_back(entity_instance->mID);

			for (auto& component_resource : entity_resource->mComponents)
			{
				const rtti::TypeInfo& instance_type = component_resource->getInstanceType();
				assert(instance_type.can_create_instance());

				entityCreationParams.mComponentToEntity.insert(std::make_pair(component_resource.get(), entity_resource));

				std::unique_ptr<ComponentInstance> component_instance(instance_type.create<ComponentInstance>({ *entity_instance, *component_resource.get() }));
				assert(component_instance);
				component_instance->mID = generateInstanceID(getInstanceID(component_resource->mID), entityCreationParams);

				entityCreationParams.mComponentInstanceMap[component_resource.get()].push_back(component_instance.get());
				entityCreationParams.mAllInstancesByID.insert(std::make_pair(component_instance->mID, component_instance.get()));
				entity_instance->addComponent(std::move(component_instance));
			}

			if (!entity_instance->init(*this, entityCreationParams, errorState))
				return false;
		}

		return true;
	}


	bool ResourceManager::initEntities(const RTTIObjectGraph& objectGraph, const ObjectByIDMap& objectsToUpdate, utility::ErrorState& errorState)
	{
		std::vector<const Entity*> entities_to_spawn;			
		objectGraph.visitNodes([&entities_to_spawn](const RTTIObjectGraph::Node& node) 
		{
			Entity* entity = rtti_cast<Entity>(node.mItem.mObject);
			if (entity != nullptr && entity->mAutoSpawn)
				entities_to_spawn.emplace_back(entity);			
		});

		EntityCreationParameters entityCreationParams(objectGraph);
		std::vector<std::string> generated_ids;
		if (!createEntities(entities_to_spawn, entityCreationParams, generated_ids, errorState))
			return false;

		if (!errorState.check(sResolveComponentPointers(entityCreationParams, errorState), "Unable to resolve pointers in components"))
			return false;

		if (!errorState.check(sInitComponents(entityCreationParams, errorState), "Unable to init components!"))
			return false;

		// Start with an empty root and add all entities without a parent to the root
		mRootEntity->clearChildren();
		for (auto& kvp : entityCreationParams.mEntityInstancesByID)
		{
			if (kvp.second->getParent() == nullptr)
				mRootEntity->addChild(*kvp.second);
		}

		patchObjectPtrs(entityCreationParams.mAllInstancesByID);

		// Replace entities currently in the resource manager with the new set
		mEntities = std::move(entityCreationParams.mEntityInstancesByID);
		
		return true;
	}


	bool ResourceManager::loadFile(const std::string& filename, const std::string& externalChangedFile, utility::ErrorState& errorState)
	{
		// ExternalChangedFile should only be used if it's different from the file being reloaded
		assert(utility::toComparableFilename(filename) != utility::toComparableFilename(externalChangedFile));

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
		// that the pointers are pointing to after loading. These would hol3d the ID, so that comparisons could be made easier.
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
				RTTIObject* object = findObject(object_to_init).get();
				assert(object);

				std::unique_ptr<RTTIObject> cloned_object = rtti::cloneObject(*object, getFactory());
				
				// TEMP HACK: Replace original object in object graph with the cloned version. This fixes problems when real-time editing components. 
				// This should be replaced with a rebuild of the object graph, but that change is on another branch
				RTTIObjectGraph::Node* originalObjectNode = object_graph.findNode(object->mID);
				assert(originalObjectNode && originalObjectNode->mItem.mType == RTTIObjectGraphItem::EType::Object);

				originalObjectNode->mItem.mObject = cloned_object.get();
				objects_to_update.emplace(std::make_pair(cloned_object->mID, std::move(cloned_object)));				
			}
		}

		// Patch again to update pointers to objects that were cloned
		patchObjectPtrs(objects_to_update);

		// init all objects in the correct order
		if (!initObjects(objects_to_init, objects_to_update, errorState))
			return false;

		// Init all entities
		if (!initEntities(object_graph, objects_to_update, errorState))
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
							nap::Logger::warn("Failed to reload %s:\n %s. \n\n See log for more information.", source_file.c_str(), errorState.toString().c_str());
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


	const ObjectPtr<RTTIObject> ResourceManager::findObject(const std::string& id)
	{
		const auto& it = mObjects.find(id);
		
		if (it != mObjects.end())
			return ObjectPtr<RTTIObject>(it->second.get());
		
		return nullptr;
	}


	void ResourceManager::addObject(const std::string& id, std::unique_ptr<RTTIObject> object)
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


	const ObjectPtr<RTTIObject> ResourceManager::createObject(const rtti::TypeInfo& type)
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


	const ObjectPtr<EntityInstance> ResourceManager::findEntity(const std::string& inID) const
	{
		EntityByIDMap::const_iterator pos = mEntities.find(getInstanceID(inID));
		if (pos == mEntities.end())
			return nullptr;

		return pos->second.get();
	}

}
