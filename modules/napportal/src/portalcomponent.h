/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "portalservice.h"
#include "portalwebsocketcomponent.h"

// External Includes
#include <component.h>
#include <componentptr.h>

namespace nap
{
	// Forward declares
	class PortalComponentInstance;

	/**
	 * Handles communication between the NAP application and a web portal.
	 */
	class NAPAPI PortalComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(PortalComponent, PortalComponentInstance)

	public:

		ComponentPtr<PortalWebSocketComponent> mPortalWebSocketComponent;	///< Property: 'PortalWebSocketComponent' the portal WebSocket component that can forward messages to this portal
	};


	/*
	 * Handles communication between the NAP application and a web portal.
	 */
	class NAPAPI PortalComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)

	public:

		/**
		 * Constructor
		 */
		PortalComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource) { }

		/**
		 * Destructor
		 */
		virtual ~PortalComponentInstance();

		/**
		 * Initializes the portal component instance
		 * @param errorState should hold the error message when initialization fails
		 * @return if the component initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * The portal WebSocket component that can forward messages to this portal
		 */
		ComponentInstancePtr<PortalWebSocketComponent> mPortalWebSocketComponent = { this, &PortalComponent::mPortalWebSocketComponent };

	private:

		PortalService* mService = nullptr;	///< Handle to the portal service.
	};
}
