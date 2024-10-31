/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "portalcomponent.h"
#include "portalutils.h"

 // External Includes
#include <entity.h>
#include <nap/core.h>
#include <nap/logger.h>

// nap::PortalComponent run time class definition
RTTI_BEGIN_CLASS(nap::PortalComponent, "Handles communication between the NAP application and a web portal")
	RTTI_PROPERTY("Server", &nap::PortalComponent::mServer, nap::rtti::EPropertyMetaData::Required, "The Portal WebSocket server to listen to")
	RTTI_PROPERTY("Items", &nap::PortalComponent::mItems, nap::rtti::EPropertyMetaData::Embedded, "All available portal items")
RTTI_END_CLASS

// nap::PortalComponentInstance run time class definition
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::PortalComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	bool PortalComponentInstance::init(utility::ErrorState& error)
	{
		// Register with the service
		mService = getEntityInstance()->getCore()->getService<nap::PortalService>();
		assert(mService != nullptr);
		mService->registerComponent(*this);

		// Store pointer to portal WebSocket server
		mServer = getComponent<PortalComponent>()->mServer.get();

		// Store pointers to portal items
		std::vector<ResourcePtr<PortalItem>>& items = getComponent<PortalComponent>()->mItems;
		for (const auto& item : items)
		{
			mItems.emplace_back(item.get());
			mItemMap.emplace(std::make_pair(item->mID, item.get()));
			item->updateSignal.connect(mItemUpdateSlot);
		}

		return true;
	}


	void PortalComponentInstance::onDestroy()
	{
		// De-register with the service
		assert(mService != nullptr);
		mService->removeComponent(*this);

		// Disconnect from portal item updates
		for (const auto& item : mItems)
			item->updateSignal.disconnect(mItemUpdateSlot);
	}


	bool PortalComponentInstance::processRequest(PortalEvent& event, utility::ErrorState& error)
	{
		// Check if the portal event has a WebSocket connection
		if (!error.check(event.hasConnection(), "%s: event is missing connection for response", getComponent()->mID.c_str()))
			return false;

		// Create response event
		PortalEventHeader res_header = { event.getID(), event.getPortalID(), EPortalEventType::Response };
		PortalEventPtr response = std::make_unique<PortalEvent>(res_header);

		// Add portal item descriptors
		for (const auto& item : mItems)
			response->addAPIEvent(item->getDescriptor());

		// Send the response to the requesting client
		return mServer->send(std::move(response), event.getConnection(), error);
	}


	bool PortalComponentInstance::processUpdate(PortalEvent& event, utility::ErrorState& error)
	{
		// Create a new portal event to notify other clients of the update
		PortalEventHeader portal_header = { event.getID(), event.getPortalID(), EPortalEventType::Update };
		PortalEventPtr portal_event = std::make_unique<PortalEvent>(portal_header);

		// Try to pass each API event to a portal item
		for (const auto& api_event : event.getAPIEvents())
		{
			// Continue with the next item when not found
			const std::string& portal_item_id = api_event->getID();
			if (!error.check(mItemMap.count(portal_item_id) == 1, "%s: does not contain portal item %s", getComponent()->mID.c_str(), portal_item_id.c_str()))
				continue;

			// Update the item and add value to portal event if successful
			const auto& item = mItemMap.at(portal_item_id);
			if (item->processUpdate(*api_event, error))
				portal_event->addAPIEvent(item->getValue());
		}

		// Broadcast update to connected clients
		mServer->broadcast(std::move(portal_event), error);

		// Done, check if we had errors
		return !error.hasErrors();
	}


	void nap::PortalComponentInstance::onItemUpdate(const PortalItem& item)
	{
		// Create the portal event for the portal item update
		const std::string& portal_id = getComponent()->mID;
		PortalEventHeader portal_header = { item.mID, portal_id, EPortalEventType::Update };
		PortalEventPtr portal_event = std::make_unique<PortalEvent>(portal_header);
		portal_event->addAPIEvent(std::move(item.getValue()));

		// Broadcast update to connected clients
		utility::ErrorState error;
		if (!mServer->broadcast(std::move(portal_event), error))
			nap::Logger::error("%s: failed to broadcast portal item update: %s", portal_id.c_str(), error.toString().c_str());
	}
}
