/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "group.h"
#include <rtti/path.h>

// Group Interface
RTTI_DEFINE_BASE(nap::IGroup)

// Define (common) Resource Group
DEFINE_GROUP(nap::ResourceGroup)

namespace nap
{
	rttr::property IGroup::getProperty() const
	{
		return get_type().get_property(IGroup::propertyName());
	}
}
