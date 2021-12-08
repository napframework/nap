/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "portalitem.h"
#include "portalevent.h"
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

		ComponentPtr<PortalWebSocketComponent> mWebSocketComponent;		///< Property: 'PortalWebSocketComponent' the portal WebSocket component that can forward messages to this portal
		std::vector<ResourcePtr<PortalItem>> mItems;					///< Property: 'Items' the portal items contained by this portal component
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
		 * Receives a portal event from the portal service to be processed
		 * @param event the portal event that is to be processed
		 * @param error contains information when processing fails
		 * @return if the event was processed successfully
		 */
		bool processEvent(PortalEventPtr event, utility::ErrorState& error);

		/**
		 * Processes a request type portal event
		 * @param event the portal event that is to be processed
		 * @param error contains information when processing fails
		 * @return if the event was processed successfully
		 */
		bool processRequestEvent(PortalEventPtr event, utility::ErrorState& error);

		/**
		 * Processes an update type portal event
		 * @param event the portal event that is to be processed
		 * @param error contains information when processing fails
		 * @return if the event was processed successfully
		 */
		bool processUpdateEvent(PortalEventPtr event, utility::ErrorState& error);

		/**
		 * The portal WebSocket component that can forward messages to this portal
		 */
		ComponentInstancePtr<PortalWebSocketComponent> mWebSocketComponent = { this, &PortalComponent::mWebSocketComponent };

	private:

		PortalService* mService = nullptr;						///< Handle to the portal service.
		std::vector<PortalItem*> mItems;						///< The portal items contained by this portal component as vector
		std::unordered_map<std::string, PortalItem*> mItemMap;	///< The portal items contained by this portal component as unordered map
	};
}
