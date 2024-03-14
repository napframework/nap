/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "portalitemlayout.h"

// External Includes
#include <nap/resource.h>
#include <nap/resourceptr.h>
#include <nap/signalslot.h>
#include <apievent.h>

namespace nap
{
	/**
	 * Represents a single item (e.g. slider, toggle, button) in a NAP portal.
	 * Implementations are in derived classes, PortalItem only serves as a base class.
	 */
	class NAPAPI PortalItem : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
        /**
         *  Initialises the portal item, calls protected onInit internally
         * @param error contains information when initialization fails
         * @return true if the portal item was initialized successfully
         */
        bool init(utility::ErrorState& error) override final;

		/**
		 * Processes an update type API event. Implementation differs per derived class.
		 * @param event The event to be processed
		 * @param error contains information when processing fails
		 * @return if the event was processed successfully
		 */
		virtual bool processUpdate(const APIEvent& event, utility::ErrorState& error) = 0;

		/**
		 * Gets the descriptor as an API event. Implementation differs per derived class.
		 * @return the descriptor of the portal item as an API event
		 */
		virtual APIEventPtr getDescriptor() const = 0;

		/**
		 * Gets the current value as an API event. Implementation differs per derived class.
		 * @return the current value of the portal item as an API event
		 */
		virtual APIEventPtr getValue() const = 0;

        /**
         * Gets the current state as an API event. Implementation differs per derived class.
         * @return the current state of the portal item as an API event
         */
        virtual APIEventPtr getState() const;

        /**
         * Gets the display name of the portal item.
         * @return the display name of the portal item
         */
        const std::string& getDisplayName() const { return mDisplayName; }

		/**
		 * Occurs when the portal item signals connected clients of a value update
		 */
		Signal<const PortalItem&> valueUpdate;

        /**
         * Occurs when the portal item signals connected clients of a state update
         */
        Signal<const PortalItem&> stateUpdate;

        /**
         * Sets the item visible or not
         * Checks if layout is changed and pushes update if this is the case
         * @param visible the new visibility state
         */
        void setVisible(bool visible);

        /**
         * Returns if item is visible
         * @return if item is visible
         */
        bool getVisible() const;

        /**
         * Sets the item enabled or not
         * Checks if layout is changed and pushes update if this is the case
         * @param enabled the new enabled state
         */
        void setEnabled(bool enabled);

        /**
         * Returns if item is enabled
         * @return if item is enabled
         */
        bool getEnabled() const;

        /**
         * Sets the padding of the portal item
         * Checks if layout is changed and pushes update if this is the case
         * @param padding the new padding
         */
        void setPadding(const PortalItemPadding& padding);

        /**
         * Returns the padding of the portal item
         * @return the padding
         */
        const PortalItemPadding& getPadding() const;

        /**
         * Set item as selected
         * @param selected
         */
        void setSelected(bool selected);

        /**
         * Returns if item is set as selected
         * @return if item is set as selected
         */
        bool getSelected() const;

        /**
         * Sets the layout of the portal item
         * Checks if layout is changed and pushes update if this is the case
         * @param layout the new layout
         */
        void setLayout(const PortalItemLayout& layout);

        /**
         * Returns the layout of the portal item
         * @return the layout
         */
        const PortalItemLayout& getLayout() const;

        // Properties
        PortalItemLayout mDefaultLayout; ///< Property: 'StartLayout' the layout of the portal item
    protected:
        /**
         * Called from init the portal item is initialized, can be extended in derived classes
         * @param error contains information when initialization fails
         * @return true if the portal item was initialized successfully
         */
        virtual bool onInit(utility::ErrorState& error){ return true; };

        /**
         * Adds the state arguments to the API event
         * Override this function to add custom state arguments
         * @param event the API event to add the state arguments to
         */
        virtual void addStateArguments(APIEventPtr& event) const;

        std::string mDisplayName;
    private:
        PortalItemLayout mLayout;

	};
}
