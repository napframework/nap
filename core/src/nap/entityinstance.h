#pragma once


#include "utility/uniqueptrvectoriterator.h"
#include "objectptr.h"

namespace nap
{
    class Core;
	class ComponentInstance;
	class ComponentResource;
	class EntityInstance;

	/**
	 * Structure used to hold data necessary to create new instances during init
	 */
	struct EntityCreationParameters
	{
		using EntityByIDMap = std::unordered_map<std::string, std::unique_ptr<EntityInstance>>;
		using InstanceByIDMap = std::unordered_map<std::string, rtti::RTTIObject*>;

		EntityByIDMap mEntitiesByID;
		InstanceByIDMap mAllInstancesByID;
	};

	/**
	 * An EntityInstance is the runtime-instance of an EntityResource, which is read from json.
	 * It contains a list of ComponentInstances and functionality to query these components
	 */
	class EntityInstance : public rtti::RTTIObject
	{
		RTTI_ENABLE(rtti::RTTIObject)

	public:
		using ComponentList = std::vector<std::unique_ptr<ComponentInstance>>;
		using ChildList = std::vector<EntityInstance*>;
		using ComponentIterator = utility::UniquePtrVectorWrapper<ComponentList, ComponentInstance*>;
		using ComponentConstIterator = utility::UniquePtrConstVectorWrapper<ComponentList, ComponentInstance*>;

		/**
		 * The constructor
		 */
		EntityInstance(Core& core);

		/**
		 * Add a component to this entity
		 *
		 * @param component The component to add. Ownership is transfered to this entity
		 */
		void addComponent(std::unique_ptr<ComponentInstance> component);

		/**
		* Find a component of the specified type. This is an exact type match, so does not return derived types
		*
		* @param type The type of the component to find.
		* @return The found component. Null if not found.
		*/
		ComponentInstance* findComponent(const rtti::TypeInfo& type) const;

		/**
		 * Get all components of the specified type. This is not an exact type match; components of a derived type are also returned
		 *
		 * @param type The type of the component to find
		 * @param components The list of components found
		 */
		void getComponentsOfType(const rtti::TypeInfo& type, std::vector<ComponentInstance*>& components) const;

		/**
		 * Convenience template function to get all components of the specified type
		 */
		template<class T>
		void getComponentsOfType(std::vector<T*>& components) const;

		/**
		 * Check whether this entity has any of components of the specified type. This is not an exact type match; components of a derived type are also returned
		 * 
		 * @param type The type of component to check for
		 */
		bool hasComponentsOfType(const rtti::TypeInfo& type) const;

		/**
		 * Convenience template function to check whether this entity has any component of the specified type
		 */
		template<class T>
		bool hasComponentsOfType() const;

		/**
		 * Convenience template function to find a component of the specified type
		 */
		template<class T>
		T* findComponent() const;

		/**
		 * Check whether this entity has a component of the specified type. This is an exact type match; components of derived types are not considered
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
		 * Get a component of the specified type. Asserts if not found.
		 *
		 * @param type The type of component to get
		 * @return The component
		 */
		ComponentInstance& getComponent(const rtti::TypeInfo& type) const;

		/**
		 * Convenience function to get a component of the specified type
		 */
		template<class T>
		T& getComponent() const;

		/**
		 * Add a child entity to this entity. Ownership is not transfered to this entity.
		 *
		 * @param child The entity to add.
		 */
		void addChild(EntityInstance& child);

		/**
		 * Clear all children from this entity
		 */
		void clearChildren();

		/**
		 * Get all children of this entity
		 */
		const ChildList& getChildren() const;

		/**
		 * Get the parent of this entity (if any)
		 */
		EntityInstance* getParent() const;

		/**
		 * Get core
		 */
		Core* getCore() const;

		/**
		 * Get components of this entity
		 */
		ComponentIterator getComponents() { return ComponentIterator(mComponents); }
		ComponentConstIterator getComponents() const { return ComponentConstIterator(mComponents); }

	private:
		Core*			mCore;
		EntityInstance* mParent = nullptr;		// Parent of this entity
		ComponentList	mComponents;			// The components of this entity
		ChildList		mChildren;				// The children of this entity
	};


	/**
	 * An EntityResource is the static data as deserialized from json. It can be used to create an EntityInstance
	 */
	class EntityResource : public rtti::RTTIObject
	{
		RTTI_ENABLE(rtti::RTTIObject)
	public:
		using ComponentList = std::vector<ObjectPtr<ComponentResource>>;
		using EntityList = std::vector<ObjectPtr<EntityResource>>;

		/**
		 * Find component of the specified type
		 *
		 * @param type The type of component to find
		 * @return The found component. Null if not found
		 */
		ObjectPtr<ComponentResource> findComponent(const rtti::TypeInfo& type) const;

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

	public:
		ComponentList	mComponents;			// The components of this entity
		EntityList		mChildren;				// The children of this entity
		bool			mAutoSpawn = true;		// Whether this entity should be automatically instantiated after deserialization
	};

	//////////////////////////////////////////////////////////////////////////

	template<class T>
	void EntityInstance::getComponentsOfType(std::vector<T*>& components) const
	{
		const rtti::TypeInfo type = rtti::TypeInfo::get<T>();
		for (auto& component : mComponents)
			if (component->get_type().is_derived_from(type))
				components.push_back(rtti_cast<T>(component.get()));
	}

	template<class T>
	bool EntityInstance::hasComponentsOfType() const
	{
		return hasComponentsOfType(rtti::TypeInfo::get<T>());
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
	bool EntityResource::hasComponent() const
	{
		return hasComponent(rtti::TypeInfo::get<T>());
	}
}
