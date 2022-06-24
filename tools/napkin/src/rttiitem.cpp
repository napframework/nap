/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "rttiitem.h"

RTTI_DEFINE_BASE(napkin::RTTIItem)

namespace napkin
{
	RTTIItem* RTTIItem::parentItem() const
	{
		return qitem_cast(QStandardItem::parent());
	}


	napkin::RTTIItem* qitem_cast(QStandardItem* qItem)
	{
		// DEBUG CHECK
		assert(qItem == nullptr || dynamic_cast<RTTIItem*>(qItem) != nullptr);
		return static_cast<RTTIItem*>(qItem);
	}
}
