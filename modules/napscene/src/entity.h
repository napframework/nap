#pragma once

// Local Includes
#include "componentptr.h"
#include "component.h"
#include "instanceproperty.h"
#include <utility/uniqueptrvectoriterator.h>
#include <rtti/objectptr.h>

namespace nap
{
    class Core;
	class Component;
	class Entity;
	class EntityInstance;	
	class Scene;

	using EntityList = std::vector<EntityInstance*>;

	/**
	 * An EntityInstance is the runtime-instance of an Entity, which is read from json.
	 * It contains a list of ComponentInstances and functionality to query these components
	 */
	class NAPAPI EntityInstance : public rtti::RTTIObject
	{
		RTTI_ENABLE(rtti::RTTIObject)

	public:
        using rtti::RTTIObject::init;
        
		using ComponentList = std::vector<std::unique_ptr<ComponentInstance>>;
		using ChildList = std::vector<EntityInstance*>;
		using ComponentIterator = utility::UniquePtrVectorWrapper<ComponentList, ComponentInstance*>;
		using ComponentConstIterator = utility::UniquePtrConstVectorWrapper<ComponentList, ComponentInstance*>;

		/**
		 * The constructor
		 * @param core: the nap core instance associated with the application
		 * @param entity: the resource that was used to create this instance, this is null
		 * when there is no resource associated with the instance, for example: the root entity
		 */
		EntityInstance(Core& core, const Entity* entity);

		/**
		* Initialize this entity
		*
		* @param entityCreationParams Parameters required to create new entity instances during init
		* @param errorState The error object
		*/
		bool init(Scene& scene, EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState);

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
         * Finds the first component with the specified ID.
         * @param type The type name of the component to find.
         * @return The found component. Null if not found.
         */
        ComponentInstance* findComponentByID(const std::string& mID) const;

		/**
		* Finds the first component of the specified type. 
		* @param type The type of the component to find.
		* @return The found component. Null if not found.
		*/
		ComponentInstance* findComponent(const rtti::TypeInfo& type, rtti::ETypeCheck typeCheck = rtti::ETypeCheck::EXACT_MATCH) const;

		/**
		* Convenience template function to find the first component of the specified type 
		* @param typeCheck if the the component of type T is derived from or a direct match
		*/
		template<class T>
		T* findComponent(rtti::ETypeCheck typeCheck = rtti::ETypeCheck::EXACT_MATCH) const;

		/**
		* Check whether this entity has a component of the specified type
		* @param type The type of component to search for
		* @param typeCheck if the the component of @type is derived from or a direct match
		*/
		bool hasComponent(const rtti::TypeInfo& type, rtti::ETypeCheck typeCheck = rtti::ETypeCheck::EXACT_MATCH) const;

		/**
		* Convenience function to check whether this entity has a component of the specified type
		* @param typeCheck if the the component of type T is derived from or a direct match
		*/
		template<class T>
		bool hasComponent(rtti::ETypeCheck typeCheck = rtti::ETypeCheck::EXACT_MATCH) const;

		/**
		* Get a component of the specified type. Asserts if not found. If multiple components of the matching type exist the first one is returned
		* @param type The type of component to get
		* @return The component
		*/
		ComponentInstance& getComponent(const rtti::TypeInfo& type, rtti::ETypeCheck typeCheck = rtti::ETypeCheck::EXACT_MATCH) const;

		/**
		* Convenience function to get a component of the specified type. If multiple components of the matching type exist the first one is returned
		* Asserts if not found
		* @param typeCheck if the the component of type T is derived from or a direct match
		* @return the found component
		*/
		template<class T>
		T& getComponent(rtti::ETypeCheck typeCheck = rtti::ETypeCheck::EXACT_MATCH) const;

		/**
		 * Get all direct entity components of the specified type. 
		 * @param type The type of the component to find
		 * @param components The list of components found
		 * @param typeCheck if the the component of type T is derived from or a direct match
		 */
		void getComponentsOfType(const rtti::TypeInfo& type, std::vector<ComponentInstance*>& components, rtti::ETypeCheck typeCheck = rtti::ETypeCheck::IS_DERIVED_FROM) const;

		/**
		 * Convenience template function to get all direct child components of the specified type T
		 * @param outComponents all direct child components of type T, note that this list is not cleared before searching
		 * @param typeCheck if the child is of the exact type or derived from type T
		 */
		template<class T>
		void getComponentsOfType(std::vector<T*>& outComponents, rtti::ETypeCheck typeCheck = rtti::ETypeCheck::IS_DERIVED_FROM) const;

		/**
		 * Convenience function that returns all components of the specified type in the entity structure recursively
		 * @param all child components of type T, note that this list is not cleared before searching
		 * @param typeCheck if the child is of the exact type or derived from type T
		 */
		template<class T>
		void getComponentsOfTypeRecursive(std::vector<T*>& outComponents, rtti::ETypeCheck typeCheck = rtti::ETypeCheck::IS_DERIVED_FROM);

		/**
		 * Check whether this entity has any of components of the specified type.
		 * @param type The type of component to check for
		 * @param typeCheck if the child is of the exact type or derived from @type
		 */
		bool hasComponentsOfType(const rtti::TypeInfo& type, rtti::ETypeCheck typeCheck = rtti::ETypeCheck::IS_DERIVED_FROM) const;

		/**
		 * Convenience template function to check whether this entity has any component of the specified type
		 * typeCheckif the child is of the exact type or derived from type T
		 */
		template<class T>
		bool hasComponentsOfType(rtti::ETypeCheck typeCheck = rtti::ETypeCheck::IS_DERIVED_FROM) const;

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
		 * @param EntityInstance to remove.
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
		ComponentIterator getComponents()					{ return ComponentIterator(mComponents); }
		
		/**
		 *	@return const component iterator
		 */
		ComponentConstIterator getComponents() const		{ return ComponentConstIterator(mComponents); }

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
	 * An Entity is the static data as deserialized from json. It can be used to create an EntityInstance
	 */
	class NAPAPI Entity : public rtti::RTTIObject
	{
		RTTI_ENABLE(rtti::RTTIObject)
	public:
		using ComponentList = std::vector<rtti::ObjectPtr<Component>>;
		using EntityList = std::vector<rtti::ObjectPtr<Entity>>;

		/**
		 * Find component of the specified type
		 *
		 * @param type The type of component to find
		 * @return The found component. Null if not found
		 */
		rtti::ObjectPtr<Component> findComponent(const rtti::TypeInfo& type, rtti::ETypeCheck typeCheck = rtti::ETypeCheck::EXACT_MATCH) const;

		/**
		 * Check whether this Entity has a component of the specified type
		 *
		 * @param type The type of component to check for
		 */
		bool hasComponent(const rtti::TypeInfo& type, rtti::ETypeCheck typeCheck = rtti::ETypeCheck::EXACT_MATCH) const;

		/** 
		 * Convenience function to check whether this entity has a component of the specified type
		 */
		template<class T>
		bool hasComponent(rtti::ETypeCheck typeCheck = rtti::ETypeCheck::EXACT_MATCH) const;

	public:
		ComponentList	mComponents;			// The components of this entity
		EntityList		mChildren;				// The children of this entity
	};

	//////////////////////////////////////////////////////////////////////////

	template<class T>
	void EntityInstance::getComponentsOfType(std::vector<T*>& components, rtti::ETypeCheck typeCheck) const
	{
		const rtti::TypeInfo type = rtti::TypeInfo::get<T>();
		for (auto& component : mComponents)
			if (rtti::isTypeMatch(component->get_type(), type, typeCheck))
				components.emplace_back(rtti_cast<T>(component.get()));
	}


	template<typename T>
	void getComponentsOfTypeRecursive(nap::EntityInstance& entity, std::vector<T*>& outComponents, rtti::ETypeCheck typeCheck /*= ETypeCheck::IS_DERIVED_FROM*/)
	{
		entity.getComponentsOfType<T>(outComponents, typeCheck);
		for (auto& child : entity.getChildren())
		{
			getComponentsOfTypeRecursive<T>(*child, outComponents, typeCheck);
		}
	}


	template<class T>
	void EntityInstance::getComponentsOfTypeRecursive(std::vector<T*>& outComponents, rtti::ETypeCheck typeCheck /*= ETypeCheck::IS_DERIVED_FROM*/)
	{
		nap::getComponentsOfTypeRecursive<T>(*this, outComponents, typeCheck);
	}


	template<class T>
	bool EntityInstance::hasComponentsOfType(rtti::ETypeCheck typeCheck) const
	{
		return hasComponentsOfType(rtti::TypeInfo::get<T>(), typeCheck);
	}

	template<class T>
	T* EntityInstance::findComponent(rtti::ETypeCheck typeCheck) const
	{
		return rtti_cast<T>(findComponent(rtti::TypeInfo::get<T>(), typeCheck));
	}

	template<class T>
	bool EntityInstance::hasComponent(rtti::ETypeCheck typeCheck) const
	{
		return hasComponent(rtti::TypeInfo::get<T>(), typeCheck);
	}

	template<class T>
	T& EntityInstance::getComponent(rtti::ETypeCheck typeCheck) const
	{
		return *rtti_cast<T>(&getComponent(rtti::TypeInfo::get<T>(), typeCheck));
	}

	//////////////////////////////////////////////////////////////////////////

	template<class T>
	bool Entity::hasComponent(rtti::ETypeCheck typeCheck) const
	{
		return hasComponent(rtti::TypeInfo::get<T>(), typeCheck);
	}
}
