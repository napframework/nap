/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "portalitemdropdown.h"
#include "portalutils.h"

// External Includes
#include <apivalue.h>

RTTI_BEGIN_CLASS(nap::PortalItemDropDown)
    RTTI_PROPERTY("Parameter", &nap::PortalItemDropDown::mParameter, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////

namespace nap
{

    bool PortalItemDropDown::init(utility::ErrorState& errorState)
    {
        mParameter->itemsChanged.connect(mItemsChangedSlot);
        mParameter->indexChanged.connect(mIndexChangedSlot);

        return true;
    }


    void PortalItemDropDown::onDestroy()
    {
        mParameter->itemsChanged.disconnect(mItemsChangedSlot);
        mParameter->indexChanged.disconnect(mIndexChangedSlot);
    }


    void PortalItemDropDown::onIndexChanged(int newIndex)
    {
        updateSignal(*this);
    }


    void PortalItemDropDown::onItemsChanged(const std::vector<std::string> &newItems)
    {
        updateSignal(*this);
    }


    bool PortalItemDropDown::processUpdate(const APIEvent& event, utility::ErrorState& error)
    {
        const APIArgument* selected_index_arg = event.getArgumentByName(nap::portal::itemValueArgName);
        if (!error.check(selected_index_arg != nullptr, "%s: update event missing argument %s", mID.c_str(), nap::portal::itemValueArgName))
            return false;

        const rtti::TypeInfo selected_index_type = selected_index_arg->getValueType();
        if (!error.check(selected_index_type == RTTI_OF(int), "%s: cannot process value type %s", mID.c_str(), selected_index_type.get_name().data()))
            return false;

        mParameter->setSelectedIndex(selected_index_arg->asInt());

        return true;
    };


    APIEventPtr PortalItemDropDown::getDescriptor() const
    {
        APIEventPtr event = std::make_unique<APIEvent>(mParameter->getDisplayName(), mID);
        event->addArgument<APIString>(nap::portal::itemTypeArgName, get_type().get_name().data());
        event->addArgument<APIValue<std::vector<std::string>>>(nap::portal::dropDownItemNames, mParameter->mItems);
        event->addArgument<APIValue<int>>(nap::portal::itemValueArgName, mParameter->mSelectedIndex);

        return event;
    }


    APIEventPtr PortalItemDropDown::getValue() const
    {
        APIEventPtr event = std::make_unique<APIEvent>(mParameter->getDisplayName(), mID);
        event->addArgument<APIStringArray>(nap::portal::dropDownItemNames, mParameter->mItems);
        event->addArgument<APIInt>(nap::portal::itemValueArgName, mParameter->mSelectedIndex);

        return event;
    }
}
