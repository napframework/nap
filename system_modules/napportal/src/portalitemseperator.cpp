/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "portalitemseperator.h"
#include "portalutils.h"

// External Includes
#include <apivalue.h>

RTTI_BEGIN_CLASS(nap::PortalItemSeparator)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////

namespace nap
{
    bool PortalItemSeparator::onInit(utility::ErrorState& error)
    {
        mDisplayName = "Separator"; // "Separator" is the default name for a separator
        return true;
    }


    bool PortalItemSeparator::processUpdate(const APIEvent& event, utility::ErrorState& error)
    {
        return true;
    }


    APIEventPtr PortalItemSeparator::getDescriptor() const
    {
        // Send an "Invalid" event value with the descriptor, so the client can deduce the argument type
        std::string value = getPortalItemButtonEventString(EPortalItemButtonEvent::Invalid);
        APIEventPtr event = std::make_unique<APIEvent>(getDisplayName(), mID);
        event->addArgument<APIString>(nap::portal::itemTypeArgName, get_type().get_name().data());
        event->addArgument<APIString>(nap::portal::itemValueArgName, value);
        addStateArguments(event);
        return event;
    }


    APIEventPtr PortalItemSeparator::getValue() const
    {
        // Send an "Invalid" event value, because clients shouln't be interested in button events
        std::string value = getPortalItemButtonEventString(EPortalItemButtonEvent::Invalid);
        APIEventPtr event = std::make_unique<APIEvent>(getDisplayName(), mID);
        event->addArgument<APIString>(nap::portal::itemValueArgName, value);
        return event;
    }


    void PortalItemSeparator::addStateArguments(nap::APIEventPtr& event) const
    {
        PortalItem::addStateArguments(event);
    }
}
