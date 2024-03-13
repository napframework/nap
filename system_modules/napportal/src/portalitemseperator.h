/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "portalitem.h"
#include "portalitemlayout.h"

// External Includes
#include <apievent.h>
#include <parameterbutton.h>

namespace nap
{
    /**
     * A seperator will be displayed as a line in the portal.
     */
    class NAPAPI PortalItemSeparator : public PortalItem
    {
    RTTI_ENABLE(PortalItem)

    public:
        /**
         * Does nothing, returns true
         * @param event const reference to the APIEvent
         * @param error contains any errors
         * @return returns true
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
    protected:
        bool onInit(utility::ErrorState &error) override;

        void addStateArguments(nap::APIEventPtr &event) const override;
    };
}
