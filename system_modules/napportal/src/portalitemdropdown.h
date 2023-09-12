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
     *
     */
    class PortalItemDropDown : public PortalItem
    {
    RTTI_ENABLE(PortalItem)

    public:

        /**
         *
         * @param errorState
         * @return
         */
        bool init(utility::ErrorState& errorState) override;

        /**
         *
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

        ResourcePtr<ParameterDropDown> mParameter;	///<
    private:
        /**
         *
         */
        Slot<int> mIndexChangedSlot = { this, &PortalItemDropDown::onIndexChanged };
        void onIndexChanged(int newIndex);

        /**
         *
         */
        Slot<const std::vector<std::string>&> mItemsChangedSlot = { this, &PortalItemDropDown::onItemsChanged };
        void onItemsChanged(const std::vector<std::string>& newItems);
    };
}
