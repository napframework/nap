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
		ComponentInstance(EntityInstance& entity, Component& resource) : 
			mEntityInstance(&entity),
			mResource(&resource)
		{ 
		}

		/**
		 * Update this component
		 */
		virtual void update(double deltaTime) {}

		/**
		 * Get the entity this component belongs to
		 */
		nap::EntityInstance* getEntityInstance() const
		{
			return mEntityInstance;
		}

		/**
		 * Get the resource this component was created from
		 */
		nap::Component* getComponent() const
		{
			return mResource;
		}

		template<typename T>
		T* getComponent() const
		{
			assert(mResource->get_type().is_derived_from(rtti::TypeInfo::get<T>()));
			return static_cast<T*>(mResource);
		}

		/**
		 * Initialize this component from its resource
		 *
		 * @param resource The resource we're being instantiated from
		 * @param entityCreationParams Parameters required to create new entity instances during init
		 * @param errorState The error object
		 */
        virtual bool init(EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState);

	private:
		EntityInstance* mEntityInstance;	// The entity this component belongs to
		Component*		mResource;			// The resource this instance was created from
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
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const { }

		/** 
		 * Get the type of ComponentInstance that should be created from this Component
		 */
		virtual const rtti::TypeInfo getInstanceType() const = 0;
	};
}
