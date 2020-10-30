/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "finderpanel.h"

using namespace napkin;

PropertyDisplayItem::PropertyDisplayItem(const PropertyPath& prop) : QStandardItem(), mProp(prop)
{
	setText(QString::fromStdString(prop.toString()));
	setEditable(false);
}



FinderPanel::FinderPanel() : QWidget()
{
	mLayout.setContentsMargins(0, 0, 0, 0);
	mLayout.addWidget(&mTree);
	setLayout(&mLayout);
	mTree.setModel(&mModel);
}


void napkin::FinderPanel::setPropertyList(const QList<napkin::PropertyPath>& properties)
{
	mModel.clear();
	for (const auto& prop : properties)
		mModel.appendRow(new PropertyDisplayItem(prop));
}

nap::qt::FilterTreeView& FinderPanel::getTreeView()
{
	return mTree;
}


