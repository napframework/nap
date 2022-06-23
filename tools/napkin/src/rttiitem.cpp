/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "rttiitem.h"

RTTI_DEFINE_BASE(napkin::RTTIItem)

namespace napkin
{
	RTTIItem* RTTIItem::parentItem() const
	{
		auto parent_item = QStandardItem::parent();
		if (parent_item != nullptr)
		{
			RTTI_ITEM(parent_item) return static_cast<RTTIItem*>(QStandardItem::parent());
		}
		return nullptr;
	}

	napkin::RTTIItem* qt_item_cast(QStandardItem* qItem)
	{
		assert(qItem == nullptr || dynamic_cast<RTTIItem*>(qItem) != nullptr);
		return static_cast<RTTIItem*>(qItem);
	}
}
