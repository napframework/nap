/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "portalitem.h"
#include "portalitemlayout.h"
#include "color.h"

// External Includes
#include <apievent.h>
#include <parameterbutton.h>

namespace nap
{
    /**
     * A static label portal item, not linked to a ParameterString but has a property called "Text" that will be
     * displayed as a Label in the portal.
     */
    class NAPAPI PortalItemStaticLabel final : public PortalItem
    {
    RTTI_ENABLE(PortalItem)

    public:
        bool onInit(utility::ErrorState &error) override;

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

        /**
         * Sets the text layout
         * Checks if text layout is changed and pushes update if this is the case
         * @param layout
         */
        void setTextLayout(const PortalItemTextLayout& layout);

        /**
         * @return the text layout
         */
        const PortalItemTextLayout& getTextLayout() const;

        // properties
        std::string mText; ///< Property: "Text" The text to be displayed
        PortalItemTextLayout mDefaultTextLayout; ///< Property: "DefaultTextLayout" The layout of the text
    protected:
        void addStateArguments(nap::APIEventPtr &event) const override;
    private:
        PortalItemTextLayout mTextLayout;
    };
}
