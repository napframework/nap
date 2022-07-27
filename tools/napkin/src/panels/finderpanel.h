/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "propertypath.h"
#include "rttiitem.h"

#include <QWidget>
#include <QStandardItemModel>
#include <napqt/filtertreeview.h>

namespace napkin
{
	class PropertyDisplayItem : public RTTIItem
	{
		Q_OBJECT
	public:
		explicit PropertyDisplayItem(const PropertyPath& prop);
		const PropertyPath& getPath() const { return mProp; }

	private:
		const PropertyPath mProp;
	};


	class FinderPanel : public QWidget
	{
	public:
		FinderPanel();
		void setPropertyList(const QList<PropertyPath>& properties);
		nap::qt::FilterTreeView& getTreeView();

	private:
		QVBoxLayout mLayout;
		QStandardItemModel mModel;
		nap::qt::FilterTreeView mTree;
	};
}
