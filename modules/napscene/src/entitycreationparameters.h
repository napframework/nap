#pragma once

// Local Includes
#include <unordered_map>
#include "component.h"
#include "componentresourcepath.h"

namespace nap
{
	namespace rtti
	{
		class RTTIObject;
	}

	class Component;
	class ComponentInstance;
	class Entity;
	class EntityInstance;	

	class EntityObjectGraphItem;
	template<typename ITEM> class ObjectGraph;
	using EntityObjectGraph = ObjectGraph<EntityObjectGraphItem>;

	/**
	 * Structure used to hold the data necessary to uniquely identify and store a cloned ComponentResource.
	 */
	class ClonedComponentResource final
	{
	public:
		ClonedComponentResource() = default;
		ClonedComponentResource(const ComponentResourcePath& path, std::unique_ptr<Component> resource);

		ComponentResourcePath		mPath;			///< The path (from the root entity) to the original component that was cloned
		std::unique_ptr<Component>	mResource;		///< The cloned component resource
	};
	using ClonedComponentResourceList = std::vector<ClonedComponentResource>;
	using ClonedComponentByEntityMap = std::unordered_map<const Entity*, ClonedComponentResourceList>;

	/**
	 * Structure used to hold data necessary to create new instances during init
	 */
	struct EntityCreationParameters final
	{
		using EntityInstanceByIDMap			= std::unordered_map<std::string, std::unique_ptr<EntityInstance>>;
		using InstanceByIDMap				= std::unordered_map<std::string, rtti::RTTIObject*>;
		using EntityInstanceMap				= std::unordered_map<Entity*, std::vector<EntityInstance*>>;
		using ComponentInstanceMap			= std::unordered_map<Component*, std::vector<ComponentInstance*>>;

		EntityCreationParameters(const EntityObjectGraph& objectGraph);
		~EntityCreationParameters();

		const EntityObjectGraph*		mObjectGraph = nullptr;						///< Object graph of a single root entity and its entire subgraph
		EntityInstanceByIDMap			mEntityInstancesByID;						///< Map containing all created entity instances and their generated instance ID
		InstanceByIDMap					mAllInstancesByID;							///< Map of both Entity and Component instances and their generated instance ID
		EntityInstanceMap				mEntityInstanceMap;							///< Map from Entity resource to a list of instantiated EntityInstances
		ComponentInstanceMap			mComponentInstanceMap;						///< Map from Component resource to a list of instantiated ComponentInstances
		ClonedComponentResourceList*	mCurrentEntityClonedComponents = nullptr;	///< List of cloned components for the current root entity being created
		ComponentResourcePath			mCurrentEntityPath;							///< Path in current entity being created
	};
}
