/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "instanceproperty.h"
#include "entitycreationparameters.h"

// External Includes
#include <rtti/object.h>
#include <utility/uniqueptrmapiterator.h>
#include <rtti/factory.h>
#include <nap/resource.h>

namespace nap
{
	// Forward Declares
	class Entity;
	class SpawnedEntityInstance;
	class Core;

	/**
	 * Represent a root entity in a scene, along with its instance properties.
	 */
	class NAPAPI RootEntity final
	{
		RTTI_ENABLE()
	public:
		rtti::ObjectPtr<Entity>							mEntity;				///< Root entity to spawn
		std::vector<ComponentInstanceProperties>		mInstanceProperties;	///< The instance properties for this entity (and all of its children)
	};

	/**
	 * Container for entities. 
	 * The Scene is responsible for instantiation and management of all contained entities.
	 */
	class NAPAPI Scene : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		using EntityByIDMap = std::unordered_map<std::string, std::unique_ptr<EntityInstance>>;
		using EntityIterator = utility::UniquePtrMapWrapper<EntityByIDMap, EntityInstance*>;
		using RootEntityList = std::vector<RootEntity>;
		using InstanceByIDMap = std::unordered_map<std::string, rtti::Object*>;
		using SortedComponentInstanceList = std::vector<ComponentInstance*>;
		using SpawnedComponentInstanceMap = std::unordered_map<EntityInstance*, SortedComponentInstanceList>;

		Scene(Core& core);
		virtual ~Scene() override;

		/**
		 * Initialize the scene. Will spawn all entities contained in this scene.
		 * As soon as this is called, EntityInstances will become available
		 * and are accessible through getRootEntity() and getEntities()
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * Destroy the scene. Will call onDestroy for all ComponentInstances and EntityInstances in the scene.
		 */
		virtual void onDestroy() override;

		/**
		 * Update all entities contained in this scene
		 * @param deltaTime time in seconds between calls.
		 */
		void update(double deltaTime);

		/**
		 * Spawns an entity hierarchy.
		 * The Entity (hierarchy) can be destroyed by calling destroy with the value returned from this function.
		 * @param entity entity resource to spawn.
		 * @param errorState contains error information if the returned object is nullptr.
		 * @return The EntityInstance that was spawned on success, nullptr otherwise. 
		 */		
		SpawnedEntityInstance spawn(const Entity& entity, utility::ErrorState& errorState);

		/**
		 * Spawns an entity hierarchy.
		 * The Entity (hierarchy) can be destroyed by calling destroy with the value returned from this function.
		 * @param entity entity resource to spawn.
		 * @param instanceProperties instance properties to apply to the entity's properties
		 * @param errorState contains error information if the returned object is nullptr.
		 * @return The EntityInstance that was spawned on success, nullptr otherwise. 
		 */		
		SpawnedEntityInstance spawn(const Entity& entity, const std::vector<ComponentInstanceProperties>& instanceProperties, utility::ErrorState& errorState);

		/**
		 * Destroys a spawned Entity hierarchy.
		 * The entity must have been obtained by the spawn() function.
		 * @param entity the Entity to destroy. 
		 */		
		void destroy(SpawnedEntityInstance& entity);

		/**
		 * Update the transform hierarchy of the entities contained in this scene. 
		 * For any TransformComponent the world transform is updated.
		 * @param deltaTime time in seconds in between calls.
		 */
		void updateTransforms(double deltaTime);

		/**
		 * @return Iterator to all entity instances in this scene.
		 */
		EntityIterator getEntities() { return EntityIterator(mEntityInstancesByID); }

		/**
		 * Locate an entity in this scene with the given unique id.
		 * Note that the given id needs to match the id of an entity resource, not instance.
		 *
		 * Example:
		 *
		 *~~~~~{.cpp}
		 * 	// Fetch world and text
		 *	auto world_entity = scene->findEntity("World");
		 *	auto laser_entity = scene->findEntity("Laser");
		 *~~~~~
		 *
		 * @param inID the unique id of the entity to find.
		 * @return EntityInstance with the specified unique identifier from this scene.
		 */
		const rtti::ObjectPtr<EntityInstance> findEntity(const std::string& inID) const;

		/**
		 * @return The root EntityInstance of this scene
		 */
		const EntityInstance& getRootEntity() const		{ return *mRootEntityInstance;}

		/**
		 * @return The root EntityInstance of this scene
		 */
		EntityInstance& getRootEntity()					{ return *mRootEntityInstance;}

		/**
		 * @return The RootEntity resources in the scene.
		 */
		RootEntityList getEntityResources()				{ return mEntities; }

		/**
		 * @return The RootEntity resources in the scene as a reference.
		 */
		RootEntityList& getEntityResourcesRef()			{ return mEntities; }

	private:
		EntityInstance* createEntityInstance(const Entity& entityResource, EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState);

		/**
		 * Instantiates an EntityInstance from an Entity.
		 */
		EntityInstance* createChildEntityInstance(const Entity& entity, int childIndex, EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState);

		/**
		 * Helper for spawning entities. Used by both spawn and init functions.
		 */
		bool spawnInternal(const RootEntityList& rootEntities, const std::vector<rtti::Object*>& allObjects, bool clearChildren, std::vector<EntityInstance*>& spawnedRootEntityInstances, SortedComponentInstanceList& sortedComponentInstances, utility::ErrorState& errorState);

	public:
		RootEntityList 						mEntities;						///< List of root entities owned by the Scene

	private:
		friend class EntityInstance;

		Core*								mCore;
		std::unique_ptr<EntityInstance>		mRootEntityInstance;			///< Root entity, owned and created by this scene
		std::unique_ptr<Entity>				mRootEntityResource;			///< Root entity resource, owned and created by this scene
		EntityByIDMap						mEntityInstancesByID;			///< Holds all spawned entities
		InstanceByIDMap						mInstancesByID;					///< Holds all spawned entities & components
		ClonedComponentResourceList			mAllClonedComponents;			///< All cloned components for this entity
		SortedComponentInstanceList			mLoadedComponentInstances;		///< Sorted list of all ComponentInstances that were created during init (i.e. resource file load)
		SpawnedComponentInstanceMap			mSpawnedComponentInstanceMap;	///< Sorted list of all ComponentInstances that were spawned at runtime, grouped by the root EntityInstance they belong to.
	};

	using SceneCreator = rtti::ObjectCreator<Scene, Core>;
}
