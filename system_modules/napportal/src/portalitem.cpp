/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

 // Local Includes
#include "portalitem.h"
#include "portalutils.h"

// nap::PortalItem run time class definition
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::PortalItem)
    RTTI_PROPERTY("DefaultLayout", &nap::PortalItem::mDefaultLayout, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS


//////////////////////////////////////////////////////////////////////////

namespace nap
{
    bool PortalItem::init(utility::ErrorState& error)
    {
        mLayout = mDefaultLayout;
        return onInit(error);
    }


    APIEventPtr PortalItem::getState() const
    {
        APIEventPtr event = std::make_unique<APIEvent>(getDisplayName(), mID);
        addStateArguments(event);
        return event;
    }


    void PortalItem::addStateArguments(APIEventPtr& event) const
    {
        addLayoutArguments(event, mLayout);
    }


    void PortalItem::setLayout(const nap::PortalItemLayout &layout)
    {
        if(layout==mLayout)
            return;

        mLayout = layout;
        stateUpdate.trigger(*this);
    }


    const PortalItemLayout& PortalItem::getLayout() const
    {
        return mLayout;
    }


    void PortalItem::setVisible(bool visible)
    {
        if(mLayout.mVisible == visible)
            return;

        mLayout.mVisible = visible;
        stateUpdate.trigger(*this);
    }


    bool PortalItem::getVisible() const
    {
        return mLayout.mVisible;
    }


    void PortalItem::setEnabled(bool enabled)
    {
        if(mLayout.mEnabled == enabled)
            return;

        mLayout.mEnabled = enabled;
        stateUpdate.trigger(*this);
    }


    bool PortalItem::getEnabled() const
    {
        return mLayout.mEnabled;
    }


    void PortalItem::setPadding(const PortalItemPadding& padding)
    {
        if(padding!=mLayout.mPadding)
            return;

        mLayout.mPadding = padding;
        stateUpdate.trigger(*this);
    }


    void PortalItem::setSelected(bool selected)
    {
        if(mLayout.mSelected == selected)
            return;

        mLayout.mSelected = selected;
        stateUpdate.trigger(*this);
    }


    bool PortalItem::getSelected() const
    {
        return mLayout.mSelected;
    }


    const PortalItemPadding& PortalItem::getPadding() const
    {
        return mLayout.mPadding;
    }
}
