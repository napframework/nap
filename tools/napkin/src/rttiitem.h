/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <QObject>
#include <QStandardItem>
#include <rtti/rtti.h>

namespace napkin
{
	/**
 	 * RTTI enabled QStandardItem, base class of ALL QStandardItems in Napkin.
	 */
	class RTTIItem : public QObject, public QStandardItem
	{
		Q_OBJECT
		RTTI_ENABLE()
	public:
		/**
		 * @return parent item, nullptr if there is no parent, asserts if parent not of type RTTIItem
		 */
		RTTIItem* parentItem() const;
	};

	/**
	 * Utility function to cast a QStandardItem to an RTTI enabled QT item, of
	 * which every item in use should be derived. Asserts if the item is not an
	 * rtti enabled QTStandardItem.
	 */
	template<typename T>
	T qitem_cast(QStandardItem* qItem)
	{
		assert(qItem == nullptr || dynamic_cast<RTTIItem*>(qItem) != nullptr);
		return qobject_cast<T>(static_cast<RTTIItem*>(qItem));
	}
}
