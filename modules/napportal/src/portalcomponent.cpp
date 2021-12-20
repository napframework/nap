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
RTTI_BEGIN_CLASS(nap::PortalComponent)
RTTI_PROPERTY("Server", &nap::PortalComponent::mServer, nap::rtti::EPropertyMetaData::Required)
RTTI_PROPERTY("Items", &nap::PortalComponent::mItems, nap::rtti::EPropertyMetaData::Embedded)
RTTI_END_CLASS

// nap::PortalComponentInstance run time class definition
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::PortalComponentInstance)
RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	PortalComponentInstance::~PortalComponentInstance()
	{
		// De-register with the service
		if (mService != nullptr)
			mService->removeComponent(*this);
	}


	bool PortalComponentInstance::init(utility::ErrorState& errorState)
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
		}

		return true;
	}


	bool PortalComponentInstance::processRequest(PortalEvent& event, utility::ErrorState& error)
	{
		// Check if the portal event has a WebSocket connection
		if (!error.check(event.hasConnection(), "%s: event is missing connection for repsonse", getComponent()->mID.c_str()))
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
		// Try to pass each API event to a portal item
		for (const auto& api_event : event.getAPIEvents())
		{
			// Continue with the next item when processing fails
			const std::string& portal_item_id = api_event->getID();
			if (!error.check(mItemMap.count(portal_item_id) == 1, "%s: does not contain portal item %s", getComponent()->mID.c_str(), portal_item_id.c_str()))
				continue;

			mItemMap.at(portal_item_id)->processUpdate(*api_event, error);
		}

		return !error.hasErrors();
	}
}
