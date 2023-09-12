/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "parameterdropdown.h"

RTTI_BEGIN_CLASS(nap::ParameterDropDown)
RTTI_PROPERTY("Items", &nap::ParameterDropDown::mItems, nap::rtti::EPropertyMetaData::Default)
RTTI_PROPERTY("SelectedIndex", &nap::ParameterDropDown::mSelectedIndex, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{

    void ParameterDropDown::setValue(const Parameter& value)
    {
        const ParameterDropDown* derived_type = rtti_cast<const ParameterDropDown>(&value);
        assert(derived_type != nullptr);

        mItems = derived_type->mItems;
        mSelectedIndex = derived_type->mSelectedIndex;
    }


    void ParameterDropDown::setSelectedIndex(int selectedIndex)
    {
        if(mSelectedIndex!=selectedIndex && selectedIndex < mItems.size())
        {
            mSelectedIndex = selectedIndex;
            indexChanged.trigger(mSelectedIndex);
        }
    }


    void ParameterDropDown::setItems(const std::vector<std::string> &items)
    {
        mItems = items;
        itemsChanged.trigger(mItems);
        if(mSelectedIndex >= mItems.size())
        {
            mSelectedIndex = mItems.size() - 1;
            indexChanged.trigger(mSelectedIndex);
        }
    }
}
