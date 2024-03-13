/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "portalitem.h"

// External Includes
#include <apievent.h>
#include <parameterdropdown.h>

namespace nap
{
    /**
     * Represents a dropdown item in a NAP Portal
     */
    class PortalItemDropDown : public PortalItem
    {
    RTTI_ENABLE(PortalItem)

    public:

        /**
         * Unsubscribes from the parameter changed signal
         */
        void onDestroy() override;

        /**
         * Processes an update type API event.
         * @param event The event to be processed
         * @param error contains information when processing fails
         * @return if the event was processed successfully
         */
        virtual bool processUpdate(const APIEvent& event, utility::ErrorState& error) override;

        /**
         * @return the descriptor of the portal item as an API event
         */
        virtual APIEventPtr getDescriptor() const override;

        /**
         * @return the current value of the portal item as an API event
         */
        virtual APIEventPtr getValue() const override;

        // Properties
        ResourcePtr<ParameterDropDown> mParameter;	///< Property: 'Parameter' the parameter linked to this portal item
    protected:
        /**
         * Subscribes to the parameter changed signal
         * @param error contains the error message when initialization fails
         * @return if initialization succeeded.
         */
        bool onInit(utility::ErrorState& errorState) override;
    private:
        /**
         * The slot and callback called when index of drop down changes
         */
        Slot<int> mIndexChangedSlot = { this, &PortalItemDropDown::onIndexChanged };
        void onIndexChanged(int newIndex);

        /**
         * The slot and callback called when items of dropdown change
         */
        Slot<const std::vector<std::string>&> mItemsChangedSlot = { this, &PortalItemDropDown::onItemsChanged };
        void onItemsChanged(const std::vector<std::string>& newItems);
    };
}
