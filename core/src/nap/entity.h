#pragma once

// Local Includes
#include "utility/uniqueptrvectoriterator.h"
#include "objectptr.h"
#include "component.h"

namespace nap
{
    class Core;
	class Component;
	class Entity;
	class EntityInstance;	

	/**
	 * Structure used to hold data necessary to create new instances during init
	 */
	struct EntityCreationParameters
	{
		using EntityByIDMap			= std::unordered_map<std::string, std::unique_ptr<EntityInstance>>;
		using InstanceByIDMap		= std::unordered_map<std::string, rtti::RTTIObject*>;
		using ComponentToEntityMap	= std::unordered_map<Component*, const Entity*>;

		virtual ~EntityCreationParameters() = default;
		EntityCreationParameters() = default;

		EntityByIDMap			mEntitiesByID;
		InstanceByIDMap			mAllInstancesByID;
		ComponentToEntityMap	mComponentToEntity;
	};

	/**
	* Selects whether the type check should be an exact type match or whether
	* the type should be derived from the given type.
	*/
	enum class NAPAPI ETypeCheck : uint8_t
	{
		EXACT_MATCH,
		IS_DERIVED_FROM
	};

	/**
	* Helper function to check whether two types match, based on a comparison mode
	*/
	static inline bool isTypeMatch(const rtti::TypeInfo& typeA, const rtti::TypeInfo& typeB, ETypeCheck typeCheck)
	{
		if (typeCheck == ETypeCheck::EXACT_MATCH)
			return typeA == typeB;
		else
			return typeA.is_derived_from(typeB);
	}


	/**
	 * An EntityInstance is the runtime-instance of an Entity, which is read from json.
	 * It contains a list of ComponentInstances and functionality to query these components
	 */
	class NAPAPI EntityInstance : public rtti::RTTIObject
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
		 * Update this entity hierarchy
		 */
		void update(double deltaTime);

		/**
		 * Add a component to this entity
		 *
		 * @param component The component to add. Ownership is transfered to this entity
		 */
		void addComponent(std::unique_ptr<ComponentInstance> component);

		/**
		* Finds the first component of the specified type. 
		*
		* @param type The type name of the component to find.
		* @return The found component. Null if not found.
		*/
		ComponentInstance* findComponent(const std::string& type) const;

		/**
		* Finds the first component of the specified type. 
		*
		* @param type The type of the component to find.
		* @return The found component. Null if not found.
		*/
		ComponentInstance* findComponent(const rtti::TypeInfo& type, ETypeCheck typeCheck = ETypeCheck::EXACT_MATCH) const;

		/**
		* Convenience template function to find the first component of the specified type
		*/
		template<class T>
		T* findComponent(ETypeCheck typeCheck = ETypeCheck::EXACT_MATCH) const;

		/**
		* Check whether this entity has a component of the specified type
		*
		* @param type The type of component to check for
		*/
		bool hasComponent(const rtti::TypeInfo& type, ETypeCheck typeCheck = ETypeCheck::EXACT_MATCH) const;

		/**
		* Convenience function to check whether this entity has a component of the specified type
		*/
		template<class T>
		bool hasComponent(ETypeCheck typeCheck = ETypeCheck::EXACT_MATCH) const;

		/**
		* Get a component of the specified type. Asserts if not found.
		*
		* @param type The type of component to get
		* @return The component
		*/
		ComponentInstance& getComponent(const rtti::TypeInfo& type, ETypeCheck typeCheck = ETypeCheck::EXACT_MATCH) const;

		/**
		* Convenience function to get a component of the specified type
		*/
		template<class T>
		T& getComponent(ETypeCheck typeCheck = ETypeCheck::EXACT_MATCH) const;

		/**
		 * Get all components of the specified type. 
		 *
		 * @param type The type of the component to find
		 * @param components The list of components found
		 */
		void getComponentsOfType(const rtti::TypeInfo& type, std::vector<ComponentInstance*>& components, ETypeCheck typeCheck = ETypeCheck::IS_DERIVED_FROM) const;

		/**
		 * Convenience template function to get all components of the specified type
		 */
		template<class T>
		void getComponentsOfType(std::vector<T*>& components, ETypeCheck typeCheck = ETypeCheck::IS_DERIVED_FROM) const;

		/**
		 * Check whether this entity has any of components of the specified type
		 * 
		 * @param type The type of component to check for
		 */
		bool hasComponentsOfType(const rtti::TypeInfo& type, ETypeCheck typeCheck = ETypeCheck::IS_DERIVED_FROM) const;

		/**
		 * Convenience template function to check whether this entity has any component of the specified type
		 */
		template<class T>
		bool hasComponentsOfType(ETypeCheck typeCheck = ETypeCheck::IS_DERIVED_FROM) const;

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
		using ComponentList = std::vector<ObjectPtr<Component>>;
		using EntityList = std::vector<ObjectPtr<Entity>>;

		/**
		 * Find component of the specified type
		 *
		 * @param type The type of component to find
		 * @return The found component. Null if not found
		 */
		ObjectPtr<Component> findComponent(const rtti::TypeInfo& type, ETypeCheck typeCheck = ETypeCheck::EXACT_MATCH) const;

		/**
		 * Check whether this Entity has a component of the specified type
		 *
		 * @param type The type of component to check for
		 */
		bool hasComponent(const rtti::TypeInfo& type, ETypeCheck typeCheck = ETypeCheck::EXACT_MATCH) const;

		/** 
		 * Convenience function to check whether this entity has a component of the specified type
		 */
		template<class T>
		bool hasComponent(ETypeCheck typeCheck = ETypeCheck::EXACT_MATCH) const;

	public:
		ComponentList	mComponents;			// The components of this entity
		EntityList		mChildren;				// The children of this entity
		bool			mAutoSpawn = true;		// Whether this entity should be automatically instantiated after deserialization
	};

	//////////////////////////////////////////////////////////////////////////

	template<class T>
	void EntityInstance::getComponentsOfType(std::vector<T*>& components, ETypeCheck typeCheck) const
	{
		const rtti::TypeInfo type = rtti::TypeInfo::get<T>();
		for (auto& component : mComponents)
			if (isTypeMatch(component->get_type(), type, typeCheck))
				components.push_back(rtti_cast<T>(component.get()));
	}

	template<class T>
	bool EntityInstance::hasComponentsOfType(ETypeCheck typeCheck) const
	{
		return hasComponentsOfType(rtti::TypeInfo::get<T>(), typeCheck);
	}

	template<class T>
	T* EntityInstance::findComponent(ETypeCheck typeCheck) const
	{
		return rtti_cast<T>(findComponent(rtti::TypeInfo::get<T>(), typeCheck));
	}

	template<class T>
	bool EntityInstance::hasComponent(ETypeCheck typeCheck) const
	{
		return hasComponent(rtti::TypeInfo::get<T>(), typeCheck);
	}

	template<class T>
	T& EntityInstance::getComponent(ETypeCheck typeCheck) const
	{
		return *rtti_cast<T>(&getComponent(rtti::TypeInfo::get<T>(), typeCheck));
	}

	//////////////////////////////////////////////////////////////////////////

	template<class T>
	bool Entity::hasComponent(ETypeCheck typeCheck) const
	{
		return hasComponent(rtti::TypeInfo::get<T>(), typeCheck);
	}
}
