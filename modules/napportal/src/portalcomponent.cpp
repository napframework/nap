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


	bool PortalComponentInstance::processEvent(PortalEventPtr event, utility::ErrorState& error)
	{
		switch (event->getType())
		{
		case EPortalEventType::Request:
			return processRequest(std::move(event), error);

		case EPortalEventType::Update:
			return processUpdate(std::move(event), error);

		case EPortalEventType::Response:
			return error.check(false, "%s: does not handle events with type %s", getComponent()->mID.c_str(), portal::eventTypeResponse);

		case EPortalEventType::Invalid:
			return error.check(false, "%s: does not handle events with type %s", getComponent()->mID.c_str(), portal::eventTypeInvalid);

		default:
			assert(false);
			return false;
		}
	}


	bool PortalComponentInstance::processRequest(PortalEventPtr event, utility::ErrorState& error)
	{
		// Create response event
		PortalEventHeader res_header = { event->getID(), event->getPortalID(), EPortalEventType::Response };
		PortalEventPtr response = std::make_unique<PortalEvent>(res_header);

		// Add portal item descriptors
		for (const auto& item : mItems)
			response->addAPIEvent(item->getDescriptor());

		return true;
	}


	bool PortalComponentInstance::processUpdate(PortalEventPtr event, utility::ErrorState& error)
	{
		// Try to pass each API event to a portal item
		for (const auto& api_event : event->getAPIEvents())
		{
			// Don't stop when processing fails, just add an error
			const std::string& portal_item_id = api_event->getID();
			if (mItemMap.count(portal_item_id))
				mItemMap.at(portal_item_id)->processUpdate(*api_event, error);
			else
				error.fail("%s: does not contain portal item %s", getComponent()->mID.c_str(), portal_item_id.c_str());
		}

		return !error.hasErrors();
	}
}
