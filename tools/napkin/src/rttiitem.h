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
 	 * RTTI Enabled QStandardItem
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
	RTTIItem* qt_item_cast(QStandardItem* qItem);
}

// Use this macro to verify that the given QT item, which isn't RTTI enabled, is an RTTI enabled QT item
#define RTTI_ITEM(qtItem)	\
	assert(dynamic_cast<napkin::RTTIItem*>(qtItem) != nullptr);
