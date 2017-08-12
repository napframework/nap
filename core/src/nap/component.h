#pragma once

#include "rtti/rttiobject.h"
#include "nap/objectptr.h"
#include "utility/dllexport.h"

namespace nap
{
	// Forward Declares
	namespace utility
	{
		class ErrorState;
	}

	class EntityInstance;
	class Component;
	struct EntityCreationParameters;

	/**
	 * A ComponentInstance is the runtime-instance of a Component, which is read from json.
	 */
	class NAPAPI ComponentInstance : public rtti::RTTIObject
	{
		RTTI_ENABLE(rtti::RTTIObject)

	public:
        using RTTIObject::init;
        
		/**
		 * Constructor
		 */
		ComponentInstance(EntityInstance& entity) : mEntity(&entity) { }

		/**
		 * Update this component
		 */
		virtual void update(double deltaTime) {}

		/**
		 * Get the entity this component belongs to
		 */
		nap::EntityInstance* getEntity() const
		{
			return mEntity;
		}

		/**
		 * Initialize this component from its resource
		 *
		 * @param resource The resource we're being instantiated from
		 * @param entityCreationParams Parameters required to create new entity instances during init
		 * @param errorState The error object
		 */
        virtual bool init(const ObjectPtr<Component>& resource, EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState);

	private:
		EntityInstance* mEntity;	// The entity this component belongs to
	};

	///////////////////////////////////////////////////////////////////////////
	// Component
	//////////////////////////////////////////////////////////////////////////

	/**
	 * A Component is the static data that is deserialized from json. A ComponentInstance is created from it
	 */
	class NAPAPI Component : public rtti::RTTIObject
	{
		RTTI_ENABLE(rtti::RTTIObject)

	public:
		/**
		 * Get a list of all component types that this component is dependent on (i.e. must be initialized before this one)
		 */
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) { }

		/** 
		 * Get the type of ComponentInstance that should be created from this Component
		 */
		virtual const rtti::TypeInfo getInstanceType() const = 0;
	};
}
