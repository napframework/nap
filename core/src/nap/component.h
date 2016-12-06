#pragma once

// RTTI Includes
#include <rtti/rtti.h>

// Core Includes
#include <nap/attributeobject.h>


namespace nap
{
	// Forward Declares
	class Entity;
	class Core;

	/**
	@brief Component

	Represents the raw data for one aspect of an Entity, and how it interacts with the world.
	A Component labels an Entity as possessing this particular aspect. An example of
	a component is a Physics Component, enabling collision detection for an Entity
	**/

	class Component : public AttributeObject
	{
		friend Entity;
		RTTI_ENABLE_DERIVED_FROM(AttributeObject)
	public:
		// All components need to have a default constructor in order to be able to constructed in a
		// modular system or a deserialization process by the help of the rtti system.
		Component() = default;

		// This method can be overriden to provide initialization behaviour. On new creation this method is called
		// after the constructor. In a de-serialization process this method is called after the attributes of this
		// component have been assigned.
		virtual void initialize(Core& core)	{ }

		// Parent
		Entity* getParent() const;
        
        // Root entity
        Entity* getRoot();

		// Lock the component
		void lockComponent();

		// Unlock the component
		void unlockComponent();

    protected:
		std::mutex mMutex;
	};

} //< End Namespace nap

// RTTI Declares
RTTI_DECLARE_BASE(nap::Component)
