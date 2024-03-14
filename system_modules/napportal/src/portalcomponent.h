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
         * Adds a portal item to the portal component
         * The portal item added using this method is not necessarily owned by the portal component
         * When you call this method, make sure to retain the item yourself
         * @param item the portal item to be added
         * @param error contains any error information
         * @return true on succes
         */
        bool addItem(PortalItem* item, utility::ErrorState& error);

        /**
         * Inserts a portal item to the portal component at a specific index
         * index is clamped between 0 and the number of items in the portal component
         * The portal item added using this method is not necessarily owned by the portal component
         * When you call this method, make sure to retain the item yourself
         * @param item the portal item to be added
         * @param index the index at which the item is to be added
         * @param error contains any error information
         * @return true on succes
         */
        bool insertItem(PortalItem* item, int index, utility::ErrorState& error);

        /**
         * Removes a portal item from the portal component
         * @param item the portal item to be removed
         */
        void removeItem(PortalItem* item);

        void openDialog(const std::string& title, const std::string& text, const std::vector<std::string>& options, Slot<int>* callback = nullptr);

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

        bool processDialogClosed(PortalEvent& event, utility::ErrorState& error);

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
		 * Called when a portal item sends a value update event
		 * @param event the value update sent by the portal item
		 */
		virtual void onItemValueUpdate(const PortalItem& item);

        /**
         * Called when a portal item sends a state update event
         * @param event the state update sent by the portal item
         */
        virtual void onItemStateUpdate(const PortalItem& item);

		PortalService* mService = nullptr;						///< Handle to the portal service
		PortalWebSocketServer* mServer = nullptr;				///< Handle to the portal WebSocket server
		std::vector<PortalItem*> mItems;						///< The portal items contained by this portal component as vector
		std::unordered_map<std::string, PortalItem*> mItemMap;	///< The portal items contained by this portal component as unordered map

		//Slot which is called when a portal item sends a value update event
		Slot<const PortalItem&> mItemValueUpdateSlot = {this, &PortalComponentInstance::onItemValueUpdate };

        //Slot which is called when a portal item sends a state update event
        Slot<const PortalItem&> mItemStateUpdateSlot = {this, &PortalComponentInstance::onItemStateUpdate };

        //Slot which is called when the resource manager has finished loading resources
        //Makes the portal page send out a request so page can be updated
        Slot<> mPostResourcesLoadedSlot;
        void onPostResourcesLoaded();

        //
        std::unordered_map<std::string, std::unique_ptr<Signal<int>>> mDialogSignals;
	};
}
