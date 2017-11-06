#include <rtti/pythonmodule.h>
#include "resourcemanager.h"
#include "rtti/rttiutilities.h"
#include "rtti/jsonreader.h"
#include "nap/core.h"
#include "objectgraph.h"
#include "entityptr.h"
#include "componentptr.h"
#include <utility/fileutils.h>
#include "logger.h"
#include "core.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ResourceManager)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_FUNCTION("findEntity", &nap::ResourceManager::findEntity)
	RTTI_FUNCTION("findObject", (const nap::ObjectPtr<nap::rtti::RTTIObject> (nap::ResourceManager::*)(const std::string&))&nap::ResourceManager::findObject)
RTTI_END_CLASS

namespace nap
{
	using namespace rtti;


	//////////////////////////////////////////////////////////////////////////
	// ResourceManagerService::RollbackHelper
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

		enum class EType : uint8_t
		{
			Object,
			File
		};

		/**
		 * Creates a graph item.
		 * @param object Object to wrap in the item that is created.
		 */
		static const RTTIObjectGraphItem create(rtti::RTTIObject* object)
		{
			RTTIObjectGraphItem item;
			item.mType = EType::Object;
			item.mObject = object;
			
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
			std::vector<rtti::ObjectLink> object_links;
			rtti::findObjectLinks(*mObject, object_links);

			for (const rtti::ObjectLink& link : object_links)
			{
				if (link.mTarget == nullptr)
					continue;

				RTTIObjectGraphItem item;
				item.mType = EType::Object;
				item.mObject = link.mTarget;
				pointees.push_back(item);
			}

			std::vector<std::string> file_links;
			rtti::findFileLinks(*mObject, file_links);

			for (std::string& filename : file_links)
			{
				RTTIObjectGraphItem item;
				item.mType = EType::File;
				item.mFilename = filename;
				pointees.push_back(item);
			}
			
			return true;
		}
		
		EType				mType;					// Type: file or object
		std::string			mFilename;				// If type is file, contains filename
		rtti::RTTIObject*	mObject = nullptr;		// If type is object, contains object pointer
	};


	//////////////////////////////////////////////////////////////////////////
	// ComponentGraphItem
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Item class for ObjectGraph usage.
	 * Wraps a Component and a map of Type to Component, which can be used to look up components by type
	 * Uses the getDependentTypes function on Component to determine "pointees"
	 */
	class ComponentGraphItem
	{
	public:
		using Type = Component*;
		using ComponentMap = std::unordered_map<rtti::TypeInfo, std::vector<Component*>>;

		/**
		 * Creates a graph item.
		 * @param componentMap Mapping from Type to Component, used to quickly lookup components by type
		 * @param component The Component we're wrapping
		 */
		static const ComponentGraphItem create(const ComponentMap& componentMap, Component* component)
		{
			ComponentGraphItem item;
			item.mComponentMap = &componentMap;
			item.mComponent = component;

			return item;
		}

		/**
		 * @return ID of the underlying Component
		 */
		const std::string getID() const
		{
			return mComponent->mID;
		}

		/**
		 * Get the component items the wrapped Component depends on. It uses the getDependentComponents function on Component
		 * to determine what points to what.
		 *
		 * @param pointees Output parameter, contains all component items this component item "points" to
		 * @param errorState If false is returned, contains information about the error.
		 * @return true is succeeded, false otherwise.
		 */
		bool getPointees(std::vector<ComponentGraphItem>& pointees, utility::ErrorState& errorState) const
		{
			std::vector<rtti::TypeInfo> dependent_types;
			mComponent->getDependentComponents(dependent_types);

			for (rtti::TypeInfo& type : dependent_types)
			{
				ComponentMap::const_iterator dependent_component = mComponentMap->find(type);
				if (!errorState.check(dependent_component != mComponentMap->end(), "Component %s was unable to find dependent component of type %s", getID().c_str(), type.get_name().data()))
					return false;

				const std::vector<Component*> components = dependent_component->second;
				for (Component* component : components)
				{
					ComponentGraphItem item;
					item.mComponent = component;
					item.mComponentMap = mComponentMap;
					pointees.push_back(item);
				}
			}

			std::vector<rtti::ObjectLink> links;
			rtti::findObjectLinks(*mComponent, links);

			for (rtti::ObjectLink& link : links)
			{
				rtti::ResolvedRTTIPath resolved_path;
				if (!errorState.check(link.mSourcePath.resolve(mComponent, resolved_path), "Encountered link from object %s that could not be resolved: %s", mComponent->mID.c_str(), link.mSourcePath.toString().c_str()))
					return false;

				if (resolved_path.getType() == RTTI_OF(EntityPtr))
				{
					EntityPtr entity_ptr = resolved_path.getValue().convert<EntityPtr>();
					nap::Entity* target_entity_resource = entity_ptr.getResource().get();

					// Skip null targets
					if (target_entity_resource == nullptr)
						continue;

					// For EntityPtrs, add all components as pointers			
					for (ObjectPtr<Component>& component : target_entity_resource->mComponents)
					{
						ComponentGraphItem item;
						item.mComponent = component.get();
						item.mComponentMap = mComponentMap;
						pointees.push_back(item);
					}
				}
				else if (resolved_path.getType().is_derived_from(RTTI_OF(ComponentPtrBase)))
				{
					rtti::Variant value = resolved_path.getValue();
					const ComponentPtrBase& component_ptr = value.get_value<ComponentPtrBase>();
					nap::Component* target_component_resource = rtti_cast<Component>(component_ptr.getResource().get());

					// Skip null targets
					if (target_component_resource == nullptr)
						continue;

					// Add target component as pointer
					ComponentGraphItem item;
					item.mComponent = target_component_resource;
					item.mComponentMap = mComponentMap;
					pointees.push_back(item);
				}
			}

			return true;
		}

		const ComponentMap*		mComponentMap = nullptr;	// Type-to-Component mapping (passed from outside)
		Component*				mComponent = nullptr;		// The Component we're wrapping
	};

	using RTTIObjectGraph = ObjectGraph<RTTIObjectGraphItem>;
	using TypeDependencyGraph = ObjectGraph<ComponentGraphItem>;


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
	// ResourceManagerService
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

		if (!objectGraph.build(all_objects, [](rtti::RTTIObject* object) { return RTTIObjectGraphItem::create(object); }, errorState))
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


	bool ResourceManager::resolvePointers(ObjectByIDMap& objectsToUpdate, const UnresolvedPointerList& unresolvedPointers, utility::ErrorState& errorState)
	{
		for (const UnresolvedPointer& unresolved_pointer : unresolvedPointers)
		{
			// Objects in objectsToUpdate have preference over the manager's objects
			RTTIObject* target_object = nullptr;
            auto object_to_update = objectsToUpdate.find(unresolved_pointer.mTargetID);
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
		return entityCreationParams.mEntitiesByID.find(generated_ids[0])->second.get();
	}


	/**
	 * This function resolves pointers in a ComponentResource of the types EntityPtr and ComponentPtr. Although the RTTI resource pointers in EntityPtr
	 * and ComponentPtr have already been resolved by the regular RTTI pointer resolving step, this step is meant explicitly to resolve pointers
	 * to instances that are stored internally in the ComponentPtr and EntityPtr.
	 * The resolving step of entities and components is more difficult than regular objects, as the entity/component structure is mirrored into
	 * a resource structure (the static objects from json) and instances (the runtime counterpart of the resources). EntityPtr and ComponentPtr
	 * are pointers that live on the resource object as the resources need to specify what other resource they are pointing to. However, the
	 * instantiated object often needs to point to other instantiated objects. In this function, we fill in the instance pointers in EntityPtr and 
	 * ComponentPtr, so that the instance can get to the instance pointer through it's resource.
	 */
	bool ResourceManager::sResolveComponentPointers(EntityCreationParameters& entityCreationParams, std::unordered_map<Component*, ComponentInstance*>& newComponentInstances, utility::ErrorState& errorState)
	{
		// We go over all component instances and resolve all Entity & Component pointers
		for (auto kvp : newComponentInstances)
		{
			ComponentInstance* source_component_instance = kvp.second;
			Component* source_component_resource = kvp.first;

			// Iterate over all the pointers in the component resource. Note that findObjectLinks returns *all* types of pointers on the object, 
			// but we're only interested in EntityPtrs and ComponentPtrs since other pointers will have been resolved during the load.
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
					nap::Entity* target_entity_resource = entity_ptr.getResource().get();

					// Skip null targets
					if (target_entity_resource == nullptr)
						continue;

					// Only AutoSpawn resources have a one-to-one relationship between resource and instance. We do not support pointers to non-AutoSpawn objects
					if (!errorState.check(target_entity_resource->mAutoSpawn, "Encountered pointer from {%s}:%s to non-AutoSpawn entity %s. This is not supported.", source_component_resource->mID.c_str(), link.mSourcePath.toString().c_str(), target_entity_resource->mID.c_str()))
						return false;

					// Find the EntityInstance and fill it in in the EntityPtr.mInstance
					EntityByIDMap::iterator target_entity_instance = entityCreationParams.mEntitiesByID.find(getInstanceID(target_entity_resource->mID));
					assert(target_entity_instance != entityCreationParams.mEntitiesByID.end());
					entity_ptr.mInstance = target_entity_instance->second.get();

					resolved_path.setValue(entity_ptr);
				}
				else if (resolved_path.getType().is_derived_from(RTTI_OF(ComponentPtrBase)))
				{
					// RTTR does not correctly support casting between base/derived types (in both directions), if one of the types is a 'wrapper' type (i.e. ComponentPtr) and the other is not (i.e. ComponentPtrBase)
					// So, while we can get the value of the pointer as a ComponentPtrBase, RTTR can no longer convert back to the specific ComponentPtr<T> that is used in order to set the value on the property again.
					//
					// To work around this we use the get_value function on the rtti::Variant we get back from getValue. This function gives you back a *const ref* to the value stored *inside* of the variant (not the original memory location)
					// So, if we can modify that const ref, we're *actually* modifying the value stored inside the variant, which means the variant always stays of the correct type (ComponentPtr<T>), which means we can
					// re-use that same variant to call setValue again. However, since get_value returns a *const ref*, we need to cast the constness aways, since we want to fill in the instance pointer.
					rtti::Variant value = resolved_path.getValue();
					ComponentPtrBase& component_ptr = const_cast<ComponentPtrBase&>(value.get_value<ComponentPtrBase>());

					nap::Component* target_component_resource = rtti_cast<Component>(component_ptr.getResource().get());

					// Skip null targets
					if (target_component_resource == nullptr)
						continue;

					// We have the component resource, now we need to find the entity resource. We do this so we can compare entities. If the target component has the same
					// entity, the target component is a sibling of our component. The reason that we check this is because we can support pointers to sibling components
					// regardless whether the entity is manually spawned or auto spawned.
					EntityCreationParameters::ComponentToEntityMap::iterator target_entity_pos = entityCreationParams.mComponentToEntity.find(target_component_resource);
					assert(target_entity_pos != entityCreationParams.mComponentToEntity.end());
					const nap::Entity* target_component_entity = target_entity_pos->second;

					// If the source & target entity are the same, we're dealing with an 'internal' component link. We can resolve the link directly against the components in the target entity instance
					EntityInstance* source_entity = source_component_instance->getEntityInstance();
					if (source_entity->getEntity() == target_component_entity)
					{
						// Find ComponentInstance with matching component resource
						for (ComponentInstance* target_component_instance : source_entity->getComponents())
						{
							if (target_component_instance->getComponent() == target_component_resource)
							{
								component_ptr.mInstance = target_component_instance;
								break;
							}
						}

						if (!errorState.check(component_ptr.mInstance != nullptr, "Failed to resolve internal ComponentPtr from {%s}:%s. Matching ComponentInstance not found.", source_component_resource->mID.c_str(), link.mSourcePath.toString().c_str()))
							return false;
					}
					else
					{
						// Source & target entity are not the same; we're dealing with an 'external' component pointer.
						// Only AutoSpawn resources have a one-to-one relationship between resource and instance. We do not support external component pointers to components in non-AutoSpawn objects
						if (!errorState.check(target_component_entity->mAutoSpawn, "Encountered pointer from {%s}:%s to non-AutoSpawn entity %s. This is not supported.", source_component_resource->mID.c_str(), link.mSourcePath.toString().c_str(), target_component_resource->mID.c_str()))
							return false;

						// Find target component instance by ID
						InstanceByIDMap::iterator target_component_instance = entityCreationParams.mAllInstancesByID.find(getInstanceID(target_component_resource->mID));
						assert(target_component_instance != entityCreationParams.mAllInstancesByID.end());
						assert(target_component_instance->second->get_type().is_derived_from<ComponentInstance>());
						component_ptr.mInstance = static_cast<ComponentInstance*>(target_component_instance->second);
					}

					// Set value back to source
					resolved_path.setValue(value);
				}
			}
		}

		return true;
	}


	static bool sInitComponents(const std::vector<const nap::Entity*>& entityResources, EntityCreationParameters& entityCreationParams, std::unordered_map<Component*, ComponentInstance*>& newComponentInstances, utility::ErrorState& errorState)
	{
		TypeDependencyGraph graph;

		std::vector<Component*> all_components;
		std::unordered_map<rtti::TypeInfo, std::vector<Component*>> components_by_type;
		for (const nap::Entity* entity_resource : entityResources)
		{
			for (auto& component : entity_resource->mComponents)
			{
				all_components.push_back(component.get());
				addComponentsByType(components_by_type, component.get(), component->get_type());
			}
		}

		auto creation_function = [&components_by_type](Component* component) {
			return ComponentGraphItem::create(components_by_type, component);
		};

		if (!graph.build(all_components, creation_function, errorState))
			return false;

		std::vector<TypeDependencyGraph::Node*> sorted_nodes = graph.getSortedNodes();

		for (TypeDependencyGraph::Node* node : sorted_nodes)
		{
			auto pos = newComponentInstances.find(node->mItem.mComponent);

			// It's possible for there to be objects in the graph that were not created during this createEntities call,
			// in the case where a component has ComponentPtrs to components of other entities that have been spawned in a previous iteration
			if (pos == newComponentInstances.end())
				continue;

			if (!pos->second->init(entityCreationParams, errorState))
				return false;
		}

		return true;
	}


	bool ResourceManager::createEntities(const std::vector<const nap::Entity*>& entityResources, EntityCreationParameters& entityCreationParams, std::vector<std::string>& generatedEntityIDs, utility::ErrorState& errorState)
	{
		std::unordered_map<Component*, ComponentInstance*> new_component_instances;

		// Create all entity instances and component instances
		for (const nap::Entity* entity_resource : entityResources)
		{
			EntityInstance* entity_instance = new EntityInstance(mCore, entity_resource);
			entity_instance->mID = generateInstanceID(getInstanceID(entity_resource->mID), entityCreationParams);

			entityCreationParams.mEntitiesByID.emplace(std::make_pair(entity_instance->mID, std::unique_ptr<EntityInstance>(entity_instance)));
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

				new_component_instances.insert(std::make_pair(component_resource.get(), component_instance.get()));
				entityCreationParams.mAllInstancesByID.insert(std::make_pair(component_instance->mID, component_instance.get()));
				entity_instance->addComponent(std::move(component_instance));
			}
		}

		
		// Now that all entities are created, make sure that parent-child relations are set correctly
		for (const nap::Entity* entity_resource : entityResources)
		{
			EntityByIDMap::iterator entity_instance = entityCreationParams.mEntitiesByID.find(getInstanceID(entity_resource->mID));
			assert(entity_instance != entityCreationParams.mEntitiesByID.end());

			for (const ObjectPtr<Entity>& child_entity_resource : entity_resource->mChildren)
			{
				EntityByIDMap::iterator child_entity_instance = entityCreationParams.mEntitiesByID.find(getInstanceID(child_entity_resource->mID));
				assert(child_entity_instance != entityCreationParams.mEntitiesByID.end());
				entity_instance->second->addChild(*child_entity_instance->second);
			}
		}

		if (!errorState.check(sResolveComponentPointers(entityCreationParams, new_component_instances, errorState), "Unable to resolve pointers in components"))
			return false;

		return sInitComponents(entityResources, entityCreationParams, new_component_instances, errorState);
	}


	bool ResourceManager::initEntities(const RTTIObjectGraph& objectGraph, const ObjectByIDMap& objectsToUpdate, utility::ErrorState& errorState)
	{
		// Build list of all entities we need to update. We need to use the objects in objectsToUpdate over those already in the ResourceManager
		// In essence, objectsToUpdate functions as an 'overlay' on top of the ResourceManager
		std::unordered_map<std::string, rtti::RTTIObject*> entities_to_spawn;
		
		// First add all EntityResources in the list of objects to update
		for (auto& kvp : objectsToUpdate)
		{
			if (kvp.second->get_type() != RTTI_OF(Entity))
				continue;

			Entity* resource = rtti_cast<Entity>(kvp.second.get());
			if (resource->mAutoSpawn)
				entities_to_spawn.insert(std::make_pair(resource->mID, resource));
		}

		// Next, go through all EntityResources currently in the resource manager and add them if they're not in the list of objects to update
		for (auto& kvp : mObjects)
		{
			if (kvp.second->get_type() != RTTI_OF(Entity))
				continue;

			ObjectByIDMap::const_iterator object_to_update = objectsToUpdate.find(kvp.first);
			if (object_to_update == objectsToUpdate.end())
			{
				Entity* resource = rtti_cast<Entity>(kvp.second.get());
				if (resource->mAutoSpawn)
					entities_to_spawn.insert(std::make_pair(resource->mID, resource));
			}
		}

		// Traverse the object graph and sort all entities based on their dependencies
		// This is determined based on an object graph traversal.
		std::vector<std::string> sorted_entity_ids_to_spawn;
		traverseAndSortIncomingObjects(entities_to_spawn, objectGraph, sorted_entity_ids_to_spawn);
		
		// Use the object IDs that were found to create a vector of objects
		std::vector<const Entity*> sorted_entities_to_spawn;
		for (const std::string& id : sorted_entity_ids_to_spawn)
		{
			auto pos = entities_to_spawn.find(id);

			// We can also encounter non-entity objects in the object graph, so we ignore those
			if (pos == entities_to_spawn.end())
				continue;

			assert(pos->second->get_type().is_derived_from<Entity>());

			sorted_entities_to_spawn.push_back(rtti_cast<Entity>(pos->second));
		}

		std::vector<std::string> generated_ids;
		EntityCreationParameters entityCreationParams;
		if (!createEntities(sorted_entities_to_spawn, entityCreationParams, generated_ids, errorState))
			return false;

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
