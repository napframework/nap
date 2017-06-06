#pragma once

#include <assert.h>
#include <memory>
#include <list>
#include <nap/core.h>
#include <nap/attributeobject.h>
#include <nap/signalslot.h>
#include <rtti/rtti.h>
#include "utility/stringutils.h"
#include "objectptr.h"

namespace nap
{
    class Core;
	class Deserializer;
    class Component;

	class EntityInstance;
	class ComponentResource;
	class ComponentInstance
	{
		RTTI_ENABLE()

	public:
		ComponentInstance(EntityInstance& entity) :
			mEntity(&entity)
		{
		}

		nap::EntityInstance* getEntity() const
		{
			return mEntity;
		}

		virtual bool init(const ObjectPtr<ComponentResource>& resource, utility::ErrorState& errorState)
		{
			return true;
		}

	private:
		EntityInstance* mEntity;
	};

	class EntityInstance
	{
	public:
		using ComponentList = std::vector<std::unique_ptr<ComponentInstance>>;

		EntityInstance(Core& core) :
			mCore(&core)
		{
		}

		void addComponent(std::unique_ptr<ComponentInstance> inComponent)
		{
			mComponents.emplace_back(std::move(inComponent));
		}

		ComponentInstance* findComponent(const rtti::TypeInfo& inType) const
		{
			ComponentList::const_iterator pos = std::find_if(mComponents.begin(), mComponents.end(), [&](auto& element) { return element->get_type() == inType; });
			if (pos == mComponents.end())
				return nullptr;

			return pos->get();
		}

		template<class T>
		T* findComponent() const
		{ 
			return rtti_cast<T>(findComponent(rtti::TypeInfo::get<T>())); 
		}

		bool hasComponent(const rtti::TypeInfo& inType) const
		{
			return findComponent(inType) != nullptr;
		}

		template<class T>
		bool hasComponent() const
		{
			return hasComponent(rtti::TypeInfo::get<T>());
		}
		
		ComponentInstance& getComponent(const rtti::TypeInfo& inType) const
		{
			ComponentInstance* result = findComponent(inType);
			assert(result != nullptr);
			return *result;
		}

		template<class T>
		T* getComponent() const
		{
			return rtti_cast<T>(getComponent(rtti::TypeInfo::get<T>()));
		}

		Core* getCore() const
		{
			return mCore;
		}

	private:
		Core* mCore;
		ComponentList mComponents;
	};

	class ComponentResource : public rtti::RTTIObject
	{
		RTTI_ENABLE(rtti::RTTIObject)

	public:
		virtual const std::vector<rtti::TypeInfo> getDependentComponents() { return std::vector<rtti::TypeInfo>(); }
		virtual std::unique_ptr<ComponentInstance> createInstance(EntityInstance& entity, utility::ErrorState& outErrorState) = 0;
	};

	class EntityResource : public rtti::RTTIObject
	{
		RTTI_ENABLE(rtti::RTTIObject)

	public:
		std::unique_ptr<EntityInstance> createInstance(Core& core, utility::ErrorState& outErrorState)
		{
			std::unique_ptr<EntityInstance> entity_instance = std::make_unique<EntityInstance>(core);
			for (auto& componentData : mComponents)
			{
				std::unique_ptr<ComponentInstance> component_instance = componentData->createInstance(*entity_instance, outErrorState);
				if (component_instance == nullptr)
					return nullptr;

				entity_instance->addComponent(std::move(component_instance));
			}
			return entity_instance;
		}
		std::vector<ObjectPtr<ComponentResource>> mComponents;
	};



	// The entity is a general purpose object. It only consists of a unique id.
	// They "tag every coarse object as a separate item".
	// An entity carries a set of components that define its behavior
	class Entity : public AttributeObject
	{
		friend Core;
		friend Deserializer;
		RTTI_ENABLE(AttributeObject)

	public:
        Entity() = default;
        
        // Virtual destructor because of virtual methods!
        virtual ~Entity() = default;

		// Add Component of type T
		// This method also ensures the new component is registered with core
		template <typename T>
        T& addComponent(const std::string& name)
        {
            auto type = rtti::TypeInfo::get<T>();
            assert(type.template is_derived_from<Component>());
            return addChild<T>(name);
        }

		template <typename T>
        T& addComponent()
        {
			rtti::TypeInfo type = rtti::TypeInfo::get<T>();
            assert(type.is_derived_from<Component>());
            return addChild<T>(type.get_name().data());
        }

		// Add a new component of @componentType
        Component& addComponent(const rtti::TypeInfo& componentType);
		// Add a component from somewhere else, forwarding parentship to this entity
        Component& addComponent(std::unique_ptr<Component> component);
		// Remove a component from this entity
        bool removeComponent(Component& comp);
        
		// Return true if this entity has a component of type T
		template <typename T>
        bool hasComponent() const;
        
		// Returns a component with the given name, null if not found
		Component* getComponent(const std::string& name) { return getChild<Component>(name); }

		// Returns the first component with the given type
        Component* getComponentOfType(const rtti::TypeInfo& componentType);
		// Returns all registered components of this entity
        std::vector<Component*> getComponents() { return getChildrenOfType<Component>(); }

		// Returns the first component of type T
		template <typename T>
        T* getComponent();
        
        // Returns the first component of type T, if none exists, create one and return it
        template<typename T>
        T& getOrCreateComponent();
        
        // Returns all components that are kind of T
		template <typename T>
        void getComponentsOfType(std::vector<T*>& outComponents) { outComponents = getChildrenOfType<T>(); }

        // Returns all components that are kind of T
		template<typename T>
        std::vector<T*> getComponentsOfType();
        
		// Returns the root entity by traversing the tree up
		Entity* getRoot();

		// Returns core system
		Core& getCore() const { return mCore; }

		// Create a new child Entity and return a pointer to it.
        Entity& addEntity(const std::string& name);

		// Remove the specified entity. The entityRemoved signal will be emitted so observers can act on it before it
		// is destroyed.
        bool removeEntity(Entity& e) { return removeChild(e); }

		// Find a child entity by name and return it. Returns a null pointer when it hasn't been found.
        Entity* getEntity(const std::string& name) { return dynamic_cast<Entity*>(getChild(name)); }

		// Return the parent/parent of this entity.
		Entity* getParent() const { return static_cast<Entity*>(getParentObject()); }

		// Return a list of child entities of which this entity is the parent.
        std::vector<Entity*> getEntities() { return getChildrenOfType<Entity>(); }

	private:
		// Force local bookkeeping by disallowing left-wing hippie type object construction.
		Entity(Core& core) : mCore(core) {}
		Core& mCore;
	};
    
    
    // template definitions

    template <typename T>
    bool Entity::hasComponent() const {
        rtti::TypeInfo type = rtti::TypeInfo::get<T>();
        assert(type.is_derived_from<Component>());
        return hasChildOfType<T>();
    }
    
    
    template <typename T>
    T* Entity::getComponent() {
        rtti::TypeInfo type = rtti::TypeInfo::get<T>();
        assert(type.is_derived_from<Component>());
        return getChildOfType<T>();
    }
    
    
    template<typename T>
    T& Entity::getOrCreateComponent() {
        T* component = getComponent<T>();
        if (component)
            return *component;
        return addComponent<T>();
    }

    
    template<typename T>
    std::vector<T*> Entity::getComponentsOfType() {
        std::vector<T*> comps;
        getComponentsOfType<T>(comps);
        return comps;
    }
    
}
