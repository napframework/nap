/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "portalitemstaticlabel.h"
#include "portalutils.h"

// External Includes
#include <apivalue.h>

RTTI_BEGIN_CLASS(nap::PortalItemStaticLabel)
    RTTI_PROPERTY("Text", &nap::PortalItemStaticLabel::mText, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("DefaultTextLayout", &nap::PortalItemStaticLabel::mDefaultTextLayout, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////

namespace nap
{

    bool PortalItemStaticLabel::processUpdate(const APIEvent& event, utility::ErrorState& error)
    {
        return true;
    }


    bool PortalItemStaticLabel::onInit(utility::ErrorState &error)
    {
        mTextLayout = mDefaultTextLayout;
        mDisplayName = "StaticLabel"; // "StaticLabel" is the default name for a static label
        return true;
    }


    APIEventPtr PortalItemStaticLabel::getDescriptor() const
    {
        APIEventPtr event = std::make_unique<APIEvent>(getDisplayName(), mID);
        event->addArgument<APIString>(nap::portal::itemTypeArgName, get_type().get_name().data());
        event->addArgument<APIString>(nap::portal::itemValueArgName, mText);
        addStateArguments(event);

        return event;
    }


    APIEventPtr PortalItemStaticLabel::getValue() const
    {
        APIEventPtr event = std::make_unique<APIEvent>(getDisplayName(), mID);
        event->addArgument<APIString>(nap::portal::itemValueArgName, mText);

        return event;
    }


    void PortalItemStaticLabel::addStateArguments(nap::APIEventPtr& event) const
    {
        PortalItem::addStateArguments(event);
        event->addArgument<APIValue<std::vector<int>>>(nap::portal::itemColorArgName, std::vector<int>({ mTextLayout.mColor[0], mTextLayout.mColor[1], mTextLayout.mColor[2] }));
        event->addArgument<APIValue<int>>(nap::portal::itemFontWeightArgName, mTextLayout.mFontWeight);
        event->addArgument<APIValue<int>>(nap::portal::itemFontSizeArgName, mTextLayout.mFontSize);
    }


    void PortalItemStaticLabel::setTextLayout(const PortalItemTextLayout& layout)
    {
        if(layout==mTextLayout)
            return;

        mTextLayout = layout;
        stateUpdate.trigger(*this);
    }


    const PortalItemTextLayout& PortalItemStaticLabel::getTextLayout() const
    {
        return mTextLayout;
    }
}
