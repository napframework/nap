/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "rttiitem.h"

RTTI_DEFINE_BASE(napkin::RTTIItem)

namespace napkin
{
	RTTIItem* RTTIItem::parentItem() const
	{
		return qitem_cast<RTTIItem*>(QStandardItem::parent());
	}
}
