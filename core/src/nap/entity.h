#pragma once

#include <assert.h>
#include <memory>
#include <list>
#include <nap/core.h>
#include <nap/attributeobject.h>
#include <nap/signalslot.h>
#include <rtti/rtti.h>
#include "stringutils.h"

namespace nap
{
    class Core;
	class Deserializer;
    class Component;

	// The entity is a general purpose object. It only consists of a unique id.
	// They "tag every coarse object as a separate item".
	// An entity carries a set of components that define its behavior
	class Entity : public AttributeObject
	{
		friend Core;
		friend Deserializer;
		RTTI_ENABLE_DERIVED_FROM(AttributeObject)

	public:
        Entity() = default;
        
        // Virtual destructor because of virtual methods!
        virtual ~Entity() = default;

		// Add Component of type T
		// This method also ensures the new component is registered with core
		template <typename T>
        T& addComponent(const std::string& name)
        {
            auto type = RTTI::TypeInfo::get<T>();
            assert(type.template isKindOf<Component>());
            return addChild<T>(name);
        }

		template <typename T>
        T& addComponent()
        {
			RTTI::TypeInfo type = RTTI::TypeInfo::get<T>();
            assert(type.isKindOf<Component>());
            return addChild<T>(type.getName());
        }

		// Add a new component of @componentType
        Component& addComponent(const RTTI::TypeInfo& componentType);
		// Add a component from somewhere else, forwarding parentship to this entity
        Component& addComponent(std::unique_ptr<Component> component);
		// Remove a component from this entity
        bool removeComponent(Component& comp);
        
		// Return true if this entity has a component of type T
		template <typename T>
        bool hasComponent() const {
			RTTI::TypeInfo type = RTTI::TypeInfo::get<T>();
            assert(type.isKindOf<Component>());
            return hasChildOfType<T>();
        }

		// Returns a component with the given name, null if not found
		Component* getComponent(const std::string& name) { return getChild<Component>(name); }

		// Returns the first component with the given type
        Component* getComponentOfType(const RTTI::TypeInfo& componentType);
		// Returns all registered components of this entity
        std::vector<Component*> getComponents() { return getChildrenOfType<Component>(); }

		// Returns the first component of type T
		template <typename T>
        T* getComponent() {
			RTTI::TypeInfo type = RTTI::TypeInfo::get<T>();
            assert(type.isKindOf<Component>());
            return getChildOfType<T>();
        }

		// Returns all components that are kind of T
		template <typename T>
        void getComponentsOfType(std::vector<T*>& outComponents) { outComponents = getChildrenOfType<T>(); }

		template<typename T>
		std::vector<T*> getComponentsOfType() {
			std::vector<T*> comps;
			getComponentsOfType<T>(comps);
			return comps;
		}

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


}

RTTI_DECLARE_BASE(nap::Entity)