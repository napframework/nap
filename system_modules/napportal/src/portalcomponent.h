/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "portalitem.h"
#include "portalevent.h"
#include "portalservice.h"
#include "portalwebsocketserver.h"

// External Includes
#include <component.h>
#include <componentptr.h>
#include <nap/resourceptr.h>

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
		ResourcePtr<PortalWebSocketServer> mServer;		///< Property: "Server" the portal WebSocket server this component listens to for events
		std::vector<ResourcePtr<PortalItem>> mItems;	///< Property: 'Items' the portal items contained by this portal component
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
		 * Initializes the portal component instance. Connect to portal item updates.
		 * @param error should hold the error message when initialization fails
		 * @return if the component initialized successfully
		 */
		virtual bool init(utility::ErrorState& error) override;

		/**
		 * Called when the portal is destroyed. Disconnect from portal item updates.
		 */
		virtual void onDestroy() override;

		/**
		 * Processes a request type portal event
		 * @param event the portal event that is to be processed
		 * @param error contains information when processing fails
		 * @return if the event was processed successfully
		 */
		bool processRequest(PortalEvent& event, utility::ErrorState& error);

		/**
		 * Processes an update type portal event
		 * @param event the portal event that is to be processed
		 * @param error contains information when processing fails
		 * @return if the event was processed successfully
		 */
		bool processUpdate(PortalEvent& event, utility::ErrorState& error);

		/**
		 * @return the client or server this component receives events from.
		 */
		const PortalWebSocketServer& getServer() const { return *mServer; }

		/**
		 * @return the client or server this component receives events from.
		 */
		PortalWebSocketServer& getServer() { return *mServer; }

	private:

		/**
		 * Called when a portal item sends an update event
		 * @param event the update sent by the portal item
		 */
		virtual void onItemUpdate(const PortalItem& item);

		PortalService* mService = nullptr;						///< Handle to the portal service
		PortalWebSocketServer* mServer = nullptr;				///< Handle to the portal WebSocket server
		std::vector<PortalItem*> mItems;						///< The portal items contained by this portal component as vector
		std::unordered_map<std::string, PortalItem*> mItemMap;	///< The portal items contained by this portal component as unordered map

		//Slot which is called when a portal item sends an update event
		Slot<const PortalItem&> mItemUpdateSlot = { this, &PortalComponentInstance::onItemUpdate };
	};
}
