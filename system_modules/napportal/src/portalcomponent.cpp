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
			if(!addItem(item.get(), error))
                return false;
		}

        // Connect to resource manager post resources slot
        // Makes the portal page send out a request so page can be updated
        auto* resource_manager = getEntityInstance()->getCore()->getResourceManager();
        mPostResourcesLoadedSlot.setFunction([this](){ onPostResourcesLoaded(); });
        resource_manager->mPostResourcesLoadedSignal.connect(mPostResourcesLoadedSlot);

		return true;
	}


    bool PortalComponentInstance::addItem(PortalItem* item, utility::ErrorState& errorState)
    {
        if(!errorState.check(mItemMap.emplace(item->mID, item).second, "Failed to add item to map"))
            return false;

        mItems.emplace_back(item);
        item->valueUpdate.connect(mItemValueUpdateSlot);
        item->stateUpdate.connect(mItemStateUpdateSlot);
        return true;
    }


    bool PortalComponentInstance::insertItem(nap::PortalItem *item, int index, utility::ErrorState &errorState)
    {
        index = math::clamp(index, 0, static_cast<int>(mItems.size()));

        if(!errorState.check(mItemMap.emplace(item->mID, item).second, "Failed to add item to map"))
            return false;

        mItems.insert(mItems.begin() + index, item);
        item->valueUpdate.connect(mItemValueUpdateSlot);
        item->stateUpdate.connect(mItemStateUpdateSlot);
        return true;
    }


    void PortalComponentInstance::removeItem(PortalItem* item)
    {
        mItems.erase(std::remove_if(mItems.begin(),
                                    mItems.end(),
                                    [item](const ResourcePtr<PortalItem>& i) { return i.get() == item; }),
                     mItems.end());
        mItemMap.erase(item->mID);
        item->valueUpdate.disconnect(mItemValueUpdateSlot);
        item->stateUpdate.disconnect(mItemStateUpdateSlot);
    }


	void PortalComponentInstance::onDestroy()
	{
		// De-register with the service
		assert(mService != nullptr);
		mService->removeComponent(*this);

		// Disconnect from portal item updates
		for (const auto& item : mItems)
        {
            item->valueUpdate.disconnect(mItemValueUpdateSlot);
            item->stateUpdate.disconnect(mItemStateUpdateSlot);
        }

        // Disconnect from resource manager
        mPostResourcesLoadedSlot.disconnect();
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
        {
            response->addAPIEvent(item->getDescriptor());
        }

		// Send the response to the requesting client
		return mServer->send(std::move(response), event.getConnection(), error);
	}


    bool PortalComponentInstance::processDialogClosed(nap::PortalEvent &event, utility::ErrorState &error)
    {
        // get uuid
        const std::string& uuid = event.getID();

        // find selection argument
        bool found_arg = false;
        for (const auto& api_event : event.getAPIEvents())
        {
            if(api_event->getName() == portal::dialogSelectionTypeArgName)
            {
                auto *selection_arg = api_event->getArgumentByName(portal::dialogSelectionArgName);
                if (selection_arg != nullptr && selection_arg->isInt())
                {
                    int selection = selection_arg->asInt();
                    auto it = mDialogSignals.find(uuid);
                    if (it != mDialogSignals.end())
                    {
                        it->second->trigger(selection);
                        mDialogSignals.erase(it);
                    }
                    found_arg = true;
                }
            }
        }

        if(!found_arg)
            error.fail("Dialog closed event is missing the selection argument");

        // Done, check if we had errors
        return !error.hasErrors();
    }


	bool PortalComponentInstance::processUpdate(PortalEvent& event, utility::ErrorState& error)
	{
		// Create a new portal event to notify other clients of the update
		PortalEventHeader portal_header = { event.getID(), event.getPortalID(), EPortalEventType::ValueUpdate };
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
            {
                portal_event->addAPIEvent(item->getValue());
            }
		}

		// Broadcast update to connected clients
		mServer->broadcast(std::move(portal_event), error);

		// Done, check if we had errors
		return !error.hasErrors();
	}


	void nap::PortalComponentInstance::onItemValueUpdate(const PortalItem& item)
	{
		// Create the portal event for the portal item update
		const std::string& portal_id = getComponent()->mID;
		PortalEventHeader portal_header = { item.mID, portal_id, EPortalEventType::ValueUpdate };
		PortalEventPtr portal_event = std::make_unique<PortalEvent>(portal_header);
		portal_event->addAPIEvent(std::move(item.getValue()));

		// Broadcast update to connected clients
		utility::ErrorState error;
		if (!mServer->broadcast(std::move(portal_event), error))
			nap::Logger::error("%s: failed to broadcast portal item update: %s", portal_id.c_str(), error.toString().c_str());
	}


    void nap::PortalComponentInstance::onItemStateUpdate(const PortalItem& item)
    {
        // Create the portal event for the portal item update
        const std::string& portal_id = getComponent()->mID;
        PortalEventHeader portal_header = { item.mID, portal_id, EPortalEventType::StateUpdate };
        PortalEventPtr portal_event = std::make_unique<PortalEvent>(portal_header);
        portal_event->addAPIEvent(std::move(item.getState()));

        // Broadcast update to connected clients
        utility::ErrorState error;
        if (!mServer->broadcast(std::move(portal_event), error))
            nap::Logger::error("%s: failed to broadcast portal item update: %s", portal_id.c_str(), error.toString().c_str());
    }


    void nap::PortalComponentInstance::onPostResourcesLoaded()
    {
        // Create the portal event for the portal item update
        const std::string& portal_id = getComponent()->mID;
        PortalEventHeader portal_header = { math::generateUUID(), portal_id, EPortalEventType::Reload };
        PortalEventPtr portal_event = std::make_unique<PortalEvent>(portal_header);

        // Broadcast update to connected clients
        utility::ErrorState error;
        if (!mServer->broadcast(std::move(portal_event), error))
            nap::Logger::error("%s: failed to broadcast portal item update: %s", portal_id.c_str(), error.toString().c_str());
    }


    void nap::PortalComponentInstance::openDialog(const std::string& title, const std::string& text, const std::vector<std::string>& options, Slot<int>* callback)
    {
        // create uuid
        std::string uuid = math::generateUUID();

        // Create the portal event for the portal item update
        const std::string& portal_id = getComponent()->mID;
        PortalEventHeader portal_header = { uuid, portal_id, EPortalEventType::OpenDialog };
        PortalEventPtr portal_event = std::make_unique<PortalEvent>(portal_header);

        // Create the dialog event
        APIEventPtr event = std::make_unique<APIEvent>(portal::openDialogEventName, uuid);
        event->addArgument(std::make_unique<APIString>(portal::dialogTitleArgName, title));
        event->addArgument(std::make_unique<APIString>(portal::dialogContentArgName, text));
        event->addArgument(std::make_unique<APIStringArray>(portal::dialogOptionsArgName, options));
        portal_event->addAPIEvent(std::move(event));

        // connect callback to closed signal
        if(callback!= nullptr)
        {
            auto signal = std::make_unique<Signal<int>>();
            signal->connect(*callback);
            mDialogSignals.emplace(uuid, std::move(signal));
        }

        // Broadcast update to connected clients
        utility::ErrorState error;
        if (!mServer->broadcast(std::move(portal_event), error))
            nap::Logger::error("%s: failed to broadcast portal item update: %s", portal_id.c_str(), error.toString().c_str());
    }
}
