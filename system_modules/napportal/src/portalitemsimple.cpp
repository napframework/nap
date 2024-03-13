/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

 // Local Includes
#include "portalitemsimple.h"

DEFINE_PORTAL_ITEM_SIMPLE(nap::PortalItemToggle)
DEFINE_PORTAL_ITEM_SIMPLE(nap::PortalItemTextField)

RTTI_BEGIN_CLASS(nap::PortalItemTextArea)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::PortalItemLabel)
RTTI_PROPERTY("DefaultTextLayout", &nap::PortalItemLabel::mStartTextLayout, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////

namespace nap
{
    bool PortalItemLabel::onInit(utility::ErrorState &error)
    {
        mTextLayout = mStartTextLayout;
        mDisplayName = "Label"; // "Label" is the default name for a label
        return true;
    }

    void PortalItemLabel::addStateArguments(nap::APIEventPtr& event) const
    {
        PortalItem::addStateArguments(event);
        event->addArgument<APIValue<std::vector<int>>>(nap::portal::itemColorArgName, std::vector<int>({ mTextLayout.mColor[0], mTextLayout.mColor[1], mTextLayout.mColor[2] }));
        event->addArgument<APIValue<int>>(nap::portal::itemFontWeightArgName, mTextLayout.mFontWeight);
        event->addArgument<APIValue<int>>(nap::portal::itemFontSizeArgName, mTextLayout.mFontSize);
    }



    void PortalItemLabel::setTextLayout(const PortalItemTextLayout& layout)
    {
        if(layout==mTextLayout)
            return;

        mTextLayout = layout;
        stateUpdate.trigger(*this);
    }


    const PortalItemTextLayout& PortalItemLabel::getTextLayout() const
    {
        return mTextLayout;
    }
}
