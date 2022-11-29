/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "componentptr.h"
#include "component.h"
#include "instanceproperty.h"

// External Includes
#include <utility/uniqueptrvectoriterator.h>
#include <nap/resource.h>
#include <nap/resourceptr.h>

namespace nap
{
	// Forward Declares
    class Core;
	class Component;
	class Entity;
	class EntityInstance;	
	class Scene;

	// Using
	using EntityList = std::vector<EntityInstance*>;

	/**
	 * The runtime counterpart of an Entity which is used to group components and child entities.
	 * This class only works with run-time versions of both, ie: ComponentInstance and EntityInstance
	 * On construction every entity receives a reference to Core and the Entity it originated from.
	 */
	class NAPAPI EntityInstance : public rtti::Object
	{
		RTTI_ENABLE(rtti::Object)

	public:
        using rtti::Object::init;
        
		using ComponentList = std::vector<std::unique_ptr<ComponentInstance>>;
		using ChildList = std::vector<EntityInstance*>;
		using ComponentIterator = utility::UniquePtrVectorWrapper<ComponentList, ComponentInstance*>;
		using ComponentConstIterator = utility::UniquePtrConstVectorWrapper<ComponentList, ComponentInstance*>;

		/**
		 * @param core: the nap core instance associated with the application
		 * @param entity: the resource that was used to create this instance, this is null
		 * when there is no resource associated with the instance, for example: the root entity
		 */
		EntityInstance(Core& core, const Entity* entity);

		/**
		* Initialize this entity.
		* @param scene scene this entity belongs to.
		* @param entityCreationParams Parameters required to create new entity instances during init
		* @param errorState contains the error if initialization fails.
		*/
		bool init(Scene& scene, EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState);

		/**
		 * Destroy this entity
		 */
		virtual void onDestroy() override {}

		/**
		 * Update this entity hierarchy
		 */
		void update(double deltaTime);

		/**
		 * Add a component to this entity
		 * @param component The component to add. Ownership is transfered to this entity
		 */
		void addComponent(std::unique_ptr<ComponentInstance> component);

		/**
		* Finds the first component of the specified type. 
		* @param type The type name of the component to find.
		* @return The found component. Null if not found.
		*/
		ComponentInstance* findComponent(const std::string& type) const;
        
        /**
         * Finds the first component with the specified ID as declared in JSON.
         * @param identifier The name of the component to find.
         * @return the found component. Null if not found.
         */
        ComponentInstance* findComponentByID(const std::string& identifier) const;

		/**
		 * Finds the first component with the specified ID as declared in JSON as type T
		 * @param identifier The name of the component to find
		 * @return the found component. Null if not found or not derived from T
		 */
		template<class T >
		T* findComponentByID(const std::string& identifier) const;

		/**
		* Finds the first component of the specified type. 
		* @param type the type of component to find.
		* @return The found component. Null if not found.
		*/
		ComponentInstance* findComponent(const rtti::TypeInfo& type) const;

		/**
		* Convenience template function to find the first component of the specified type
		*/
		template<class T>
		T* findComponent() const;

		/**
		* Check whether this entity has a component of the specified type
		* @param type The type of component to search for
		*/
		bool hasComponent(const rtti::TypeInfo& type) const;

		/**
		* Convenience function to check whether this entity has a component of the specified type
		*/
		template<class T>
		bool hasComponent() const;

		/**
		* Get a component of the specified type. Asserts if not found. If multiple components of the matching type exist the first one is returned
		* @return The component
		*/
		ComponentInstance& getComponent(const rtti::TypeInfo& type) const;

		/**
		* Convenience function to get a component of the specified type. If multiple components of the matching type exist the first one is returned.
		* Asserts if not found.
		* @return the found component
		*/
		template<class T>
		T& getComponent() const;

		/**
		 * Get all direct entity components of the specified type. 
		 * @param type The type of the component to find
		 * @param components The list of components found
		 */
		void getComponentsOfType(const rtti::TypeInfo& type, std::vector<ComponentInstance*>& components) const;

		/**
		 * Convenience template function to get all direct child components of the specified type T
		 * @param outComponents all direct child components of type T, note that this list is not cleared before searching
		 */
		template<class T>
		void getComponentsOfType(std::vector<T*>& outComponents) const;

		/**
		 * Convenience function that returns all components of the specified type in the entity structure recursively
		 * @param outComponents all child components of type T, note that this list is not cleared before search
		 */
		template<class T>
		void getComponentsOfTypeRecursive(std::vector<T*>& outComponents);

		/**
		 * Check whether this entity has any of components of the specified type.
		 * @param type The type of component to check for
		 */
		bool hasComponentsOfType(const rtti::TypeInfo& type) const;

		/**
		 * Convenience template function to check whether this entity has any component of the specified type
		 */
		template<class T>
		bool hasComponentsOfType() const;

		/**
		 * Add a child entity to this entity. Ownership is not transfered to this entity.
		 * @param child The entity to add.
		 */
		void addChild(EntityInstance& child);

		/**
		 * Clear all children from this entity
		 */
		void clearChildren();

		/**
		 * Removes a single child from the entity instance
		 * @param entityInstance to remove.
		 */
		void removeChild(const EntityInstance& entityInstance);

		/**
		 * Get all children of this entity
		 */
		const ChildList& getChildren() const;

		/**
		 * Get the parent of this entity (if any)
		 */
		EntityInstance* getParent() const;

		/**
		 * Get the entity resource of this instance
		 * @return the entity resource associated with this instance, nullptr if no resource is associated
		 * with this entity, ie: the root
		 */
		const Entity* getEntity() const;

		/**
		 * @return core
		 */
		Core* getCore() const;

		/**
		 * @return Non const component iterator
		 */
		ComponentIterator getComponents()									{ return ComponentIterator(mComponents); }
		
		/**
		 *	@return const component iterator
		 */
		ComponentConstIterator getComponents() const						{ return ComponentConstIterator(mComponents); }

		/**
		 * 
		 * @return child entity at index. Asserts when out of range
		 */
		EntityInstance& operator[](std::size_t index)						{ assert(index < mChildren.size()); return *(mChildren[index]); }

		/**
		 * @return child entity at index. Asserts when out of range.
		 */
		const EntityInstance& operator[](std::size_t index) const			{ assert(index < mChildren.size()); return *(mChildren[index]); }

	private:
		Core*			mCore = nullptr;
		const Entity*	mResource = nullptr;	// Resource of this entity
		EntityInstance* mParent = nullptr;		// Parent of this entity
		ComponentList	mComponents;			// The components of this entity
		ChildList		mChildren;				// The children of this entity
	};

	/**
	 * A SpawnedEntityInstance is identical in functionality to a regular EntityInstance. It is only used to be able to 
	 * distinguish between entities that were spawned during init and entities that were spawned at runtime.
	 * A SpawnedEntityInstance is created through the Scene::spawn interface and can be destroyed by passing this object 
	 * to Scene::destroy.
	 */
	class NAPAPI SpawnedEntityInstance
	{
	public:
		SpawnedEntityInstance() = default;

		rtti::ObjectPtr<EntityInstance>& get() { return mEntityInstance; }

		const rtti::ObjectPtr<EntityInstance>& get() const { return mEntityInstance; }

		const EntityInstance& operator*() const
		{
			assert(mEntityInstance != nullptr);
			return *mEntityInstance;
		}

		EntityInstance& operator*()
		{
			assert(mEntityInstance != nullptr);
			return *mEntityInstance;
		}

		EntityInstance* operator->() const
		{
			assert(mEntityInstance != nullptr);
			return mEntityInstance.get();
		}

		EntityInstance* operator->()
		{
			assert(mEntityInstance != nullptr);
			return mEntityInstance.get();
		}

		bool operator==(const SpawnedEntityInstance& other) const
		{
			return mEntityInstance == other.mEntityInstance;
		}

		bool operator==(const EntityInstance* ptr) const
		{
			return mEntityInstance == ptr;
		}

		bool operator!=(const SpawnedEntityInstance& other) const
		{
			return mEntityInstance != other.mEntityInstance;
		}

		bool operator!=(const EntityInstance* ptr) const
		{
			return mEntityInstance != ptr;
		}

		bool operator<(const SpawnedEntityInstance& other) const
		{
			return mEntityInstance < other.mEntityInstance;
		}

		bool operator>(const SpawnedEntityInstance& other) const
		{
			return mEntityInstance > other.mEntityInstance;
		}

		bool operator<=(const SpawnedEntityInstance& other) const
		{
			return mEntityInstance <= other.mEntityInstance;
		}

		bool operator>=(const SpawnedEntityInstance& other) const
		{
			return mEntityInstance >= other.mEntityInstance;
		}

	private:
		friend class Scene;

		SpawnedEntityInstance(EntityInstance* entityInstance) :
			mEntityInstance(entityInstance)
		{
		}

	private:
		rtti::ObjectPtr<EntityInstance> mEntityInstance;
	};

	//////////////////////////////////////////////////////////////////////////
	// Entity Resource
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Resource part of an entity.
	 * This represents the static data that is deserialized from json and contains a list of child entities and components
	 * This class is used as a blueprint for the creation of an EntityInstance.
	 */
	class NAPAPI Entity : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		using ComponentList = std::vector<rtti::ObjectPtr<Component>>;
		using EntityList = std::vector<rtti::ObjectPtr<Entity>>;

		/**
		 * Find component of the specified type
		 *
		 * @param type The type of component to find
		 * @return The found component. Null if not found
		 */
		rtti::ObjectPtr<Component> findComponent(const rtti::TypeInfo& type) const;

		/**
		 * Check whether this Entity has a component of the specified type
		 *
		 * @param type The type of component to check for
		 */
		bool hasComponent(const rtti::TypeInfo& type) const;

		/** 
		 * Convenience function to check whether this entity has a component of the specified type
		 */
		template<class T>
		bool hasComponent() const;

		/**
		 * @return All the components currently on this entity
		 */
		const ComponentList& getComponents() const							{ return mComponents; }

		/**
		 * @return children property name
		 */
		static constexpr const char* componentsPropertyName()				{ return "Components"; }

		/**
		 * @return children property name
		 */
		static constexpr const char* childrenPropertyName()					{ return "Children"; }

	public:
		ComponentList	mComponents;			// The components of this entity
		EntityList		mChildren;				// The children of this entity
	};

	//////////////////////////////////////////////////////////////////////////

	template<class T>
	void EntityInstance::getComponentsOfType(std::vector<T*>& components) const
	{
		const rtti::TypeInfo type = rtti::TypeInfo::get<T>();
		for (auto& component : mComponents)
			if (rtti::isTypeMatch(component->get_type(), type, rtti::ETypeCheck::IS_DERIVED_FROM))
				components.emplace_back(rtti_cast<T>(component.get()));
	}


	template<typename T>
	void getComponentsOfTypeRecursive(nap::EntityInstance& entity, std::vector<T*>& outComponents)
	{
		entity.getComponentsOfType<T>(outComponents);
		for (auto& child : entity.getChildren())
			getComponentsOfTypeRecursive<T>(*child, outComponents);
	}


	template<class T>
	void EntityInstance::getComponentsOfTypeRecursive(std::vector<T*>& outComponents)
	{
		nap::getComponentsOfTypeRecursive<T>(*this, outComponents);
	}


	template<class T>
	bool EntityInstance::hasComponentsOfType() const
	{
		return hasComponentsOfType(rtti::TypeInfo::get<T>());
	}

	template<class T >
	T* EntityInstance::findComponentByID(const std::string& identifier) const
	{
		T* element = rtti_cast<T>(findComponentByID(identifier));
		if (element == nullptr)
			return nullptr;
		return rtti::isTypeMatch(element->get_type(), RTTI_OF(T), rtti::ETypeCheck::IS_DERIVED_FROM) ? element : nullptr;
	}

	template<class T>
	T* EntityInstance::findComponent() const
	{
		return rtti_cast<T>(findComponent(rtti::TypeInfo::get<T>()));
	}

	template<class T>
	bool EntityInstance::hasComponent() const
	{
		return hasComponent(rtti::TypeInfo::get<T>());
	}

	template<class T>
	T& EntityInstance::getComponent() const
	{
		return *rtti_cast<T>(&getComponent(rtti::TypeInfo::get<T>()));
	}

	//////////////////////////////////////////////////////////////////////////

	template<class T>
	bool Entity::hasComponent() const
	{
		return hasComponent(rtti::TypeInfo::get<T>());
	}
}
