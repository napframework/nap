#pragma once

#include "instanceproperty.h"
#include "entitycreationparameters.h"
#include <rtti/rttiobject.h>
#include <utility/UniquePtrMapIterator.h>
#include <rtti/factory.h>

namespace nap
{
	class Entity;
	class Core;

	/**
	 *
	 */
	class RootEntity final
	{
	public:
		ObjectPtr<Entity>							mEntity;				///< Root entity to spawn
		std::vector<ComponentInstanceProperties>	mInstanceProperties;	// The instance properties for this entity
	};

	/**
	 * 
	 */
	class NAPAPI Scene : public rtti::RTTIObject
	{
		RTTI_ENABLE(rtti::RTTIObject)

	public:
		using EntityByIDMap = std::unordered_map<std::string, std::unique_ptr<EntityInstance>>;
		using EntityIterator = utility::UniquePtrMapWrapper<EntityByIDMap, EntityInstance*>;

		Scene(Core& core);

		/**
		 *
		 */
        virtual bool init(utility::ErrorState& errorState);


		/**
		 *
		 */
		void update(double deltaTime);

		/**
		* @return Iterator to all entities in this scene.
		*/
		EntityIterator getEntities() { return EntityIterator(mEntityInstances); }

		/**
		* @return EntityInstance in this scene.
		*/
		const ObjectPtr<EntityInstance> findEntity(const std::string& inID) const;

		/**
		* @return The root entity as created by the system, which is the root parent of all entities.
		*/
		const EntityInstance& getRootEntity() const		{ return *mRootEntity;}

		/**
		* @return The root entity as created by the system, which is the root parent of all entities.
		*/
		EntityInstance& getRootEntity()		{ return *mRootEntity;}

	private:
		EntityInstance* createEntityInstance(const Entity& entityResource, EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState);

		/**
		* Instantiates an Entity.
		*/
		EntityInstance* createChildEntityInstance(const Entity& Entity, int childIndex, EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState);

		using RootEntityInstanceMap = std::unordered_map<std::string, EntityInstance*>;
		static bool sResolveComponentPointers(EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState);
		static ComponentInstance* sResolveComponentInstancePath(ComponentInstance* sourceComponentInstance, const std::string& targetComponentInstancePath, Component* targetComponentResource,
																const RootEntityInstanceMap& rootEntityInstances, const EntityCreationParameters::ComponentInstanceMap& componentInstances, utility::ErrorState& errorState);

	public:
		using RootEntityList = std::vector<RootEntity>;
		RootEntityList mEntities;

	private:
		friend class EntityInstance;

		Core*								mCore;
		std::unique_ptr<EntityInstance>		mRootEntity;					// Root entity, owned and created by the system
		EntityByIDMap						mEntityInstances;				// Holds all spawned entities
		ClonedComponentByEntityMap			mClonedComponentsByEntity;		// All cloned components, stored by entity. This map owns the cloned resources.
	};

	using SceneCreator = rtti::ObjectCreator<Scene, Core>;
}
