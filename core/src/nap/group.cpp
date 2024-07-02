/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "group.h"

// Group Interface
RTTI_DEFINE_BASE(nap::IGroup)

// Define (common) Resource Group
DEFINE_GROUP(nap::ResourceGroup, nap::Resource)

namespace nap
{

	rttr::property IGroup::getMembersProperty() const
	{
		auto prop = get_type().get_property(mMembersPropertyName.data());
		assert(prop.is_valid());
		return prop;
	}


	rttr::property IGroup::getChildrenProperty() const
	{
		auto prop = get_type().get_property(mChildrenPropertyName.data());
		assert(prop.is_valid());
		return prop;
	}
}
