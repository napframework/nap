#pragma once

#include <rtti/rtti.h>

// NAP Includes
#include <nap/attribute.h>
#include <nap/component.h>
#include <nap/signalslot.h>

namespace nap
{
	class Service;

	/**
	@brief Serviceable Component

	A specialization of a component that a service uses -> ie: client of a service
	This component automatically registers itself with a service after being attached to an entity -> receives a parent
	When destructed the component de-registers itself from a service
	**/

	class ServiceableComponent : public Component
	{
		RTTI_ENABLE_DERIVED_FROM(Component)
	public:
		ServiceableComponent();

	protected:
		// Service to which this component is a client
		Service* mService = nullptr;

		// Called when the component has been registered
		virtual void registered()	{ }

	private:
        // Slots that handle service registration / deregistration
        Slot<const Object&> mAdded = { this, &ServiceableComponent::registerWithService };
        
		// Slot Calls
		void registerWithService(const Object& object);
	};
}

RTTI_DECLARE_BASE(nap::ServiceableComponent)
