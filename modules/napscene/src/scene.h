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
	 * Represent a root entity in a scene, along with its instance properties.
	 */
	class RootEntity final
	{
	public:
		ObjectPtr<Entity>							mEntity;				///< Root entity to spawn
		std::vector<ComponentInstanceProperties>	mInstanceProperties;	//< The instance properties for this entity (and all of its children)
	};

	/**
	 * Container for entities. The Scene is responsible for instantiating all of the entities.
	 */
	class NAPAPI Scene : public rtti::RTTIObject
	{
		RTTI_ENABLE(rtti::RTTIObject)

	public:
		using EntityByIDMap = std::unordered_map<std::string, std::unique_ptr<EntityInstance>>;
		using EntityIterator = utility::UniquePtrMapWrapper<EntityByIDMap, EntityInstance*>;

		Scene(Core& core);
		virtual ~Scene() override;

		/**
		 * Initialize the scene. Will spawn all entities contained in this scene.
		 */
        virtual bool init(utility::ErrorState& errorState);

		/**
		 * Update all entities contained in this scene
		 */
		void update(double deltaTime);

		/**
		 * Update the transform hierarchy of the entities contained in this scene. For any TransformComponent the world transform is updated.
		 */
		void updateTransforms(double deltaTime);

		/**
		* @return Iterator to all entities in this scene.
		*/
		EntityIterator getEntities() { return EntityIterator(mEntityInstancesByID); }

		/**
		* @return EntityInstance in this scene.
		*/
		const ObjectPtr<EntityInstance> findEntity(const std::string& inID) const;

		/**
		* @return The root entity of this scene
		*/
		const EntityInstance& getRootEntity() const		{ return *mRootEntity;}

		/**
		* @return The root entity of this scene
		*/
		EntityInstance& getRootEntity()		{ return *mRootEntity;}

	private:
		EntityInstance* createEntityInstance(const Entity& entityResource, EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState);

		/**
		* Instantiates an Entity.
		*/
		EntityInstance* createChildEntityInstance(const Entity& Entity, int childIndex, EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState);

	public:
		using RootEntityList = std::vector<RootEntity>;
		RootEntityList mEntities;

	private:
		friend class EntityInstance;

		Core*								mCore;
		std::unique_ptr<EntityInstance>		mRootEntity;					// Root entity, owned and created by this scene
		EntityByIDMap						mEntityInstancesByID;			// Holds all spawned entities
		ClonedComponentByEntityMap			mClonedComponentsByEntity;		// All cloned components, stored by entity. This map owns the cloned resources.
	};

	using SceneCreator = rtti::ObjectCreator<Scene, Core>;
}
