/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "standarditemsgeneric.h"
#include "napkinglobals.h"

napkin::EmptyItem::EmptyItem() : QStandardItem()
{
	setEditable(false);
}

napkin::InvalidItem::InvalidItem(const QString& name) : QStandardItem(name)
{
	setForeground(Qt::red);
	setEditable(false);
}
