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

	class RTTIObjectGraphItem;
	template<typename ITEM> class ObjectGraph;
	using RTTIObjectGraph = ObjectGraph<RTTIObjectGraphItem>;

	/**
	 * Structure used to hold the data necessary to uniquely identify and store a cloned ComponentResource.
	 */
	class ClonedComponentResource final
	{
	public:
		ClonedComponentResource() = default;
		ClonedComponentResource(const std::string& path, std::unique_ptr<Component> resource);

		std::string					mPath;			///< The path (from the root entity) to the original component that was cloned
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
		using ComponentToEntityMap			= std::unordered_map<Component*, const Entity*>;
		using ComponentInstanceMap			= std::unordered_map<Component*, std::vector<ComponentInstance*>>;

		EntityCreationParameters(const RTTIObjectGraph& objectGraph);
		~EntityCreationParameters();

		const RTTIObjectGraph*			mObjectGraph = nullptr;
		EntityInstanceByIDMap			mEntityInstancesByID;
		InstanceByIDMap					mAllInstancesByID;
		ComponentInstanceMap			mComponentInstanceMap;
		ComponentToEntityMap			mComponentToEntity;
		ClonedComponentResourceList*	mCurrentEntityClonedComponents = nullptr;	///< List of cloned components for the current root entity being created
		ComponentResourcePath			mCurrentEntityPath;							///< Path in current entity being created
	};
}
