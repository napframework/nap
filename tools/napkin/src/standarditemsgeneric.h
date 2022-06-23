/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "rttiitem.h"
#include <QtGui/QStandardItem>

namespace napkin
{

	/**
	 * An empty item.
	 */
	class EmptyItem : public RTTIItem
	{
	public:
		/**
		 * Constructor
		 */
		EmptyItem();
	};


	/**
	 * An item representing invalid data
	 */
	class InvalidItem : public RTTIItem
	{
	public:
		/**
		 * @param name Text to be displayed
		 */
		explicit InvalidItem(const QString& name);
	};

}; // napkin
