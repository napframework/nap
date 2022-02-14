/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "modulepanel.h"
#include "napkin-resources.h"
#include <appcontext.h>

using namespace napkin;

ModuleItem::ModuleItem(const nap::Module& module)
		: QStandardItem(), mModule(module)
{
	std::string name(module.getDescriptor().mID);
	setText(QString::fromStdString(name));
	setEditable(false);
}


QVariant napkin::ModuleItem::data(int role) const
{
	switch (role)
	{
	case Qt::DecorationRole:
		return AppContext::get().getResourceFactory().getIcon(QRC_ICONS_MODULE);
	default:
		return QStandardItem::data(role);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ModuleModel::ModuleModel() : QStandardItemModel()
{
	auto ctx = &AppContext::get();
	connect(ctx, &AppContext::coreInitialized, this, &ModuleModel::onCoreInitialized);
	setHorizontalHeaderLabels({"Name"});
}


void ModuleModel::onCoreInitialized()
{
	removeRows(0, rowCount());
	nap::Core& core = AppContext::get().getCore();
	assert(core.isInitialized());
	for (const auto& mod : core.getModuleManager().getModules())
	{
		appendRow(new ModuleItem(*mod));
	}
	populated();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ModulePanel::ModulePanel() : QWidget()
{
	setLayout(&mLayout);
	mLayout.setContentsMargins(0, 0, 0, 0);
	mLayout.addWidget(&mTreeView);
	mTreeView.setModel(&mModel);
	mTreeView.enableSorting();
	connect(&mModel, &ModuleModel::populated, [&]()
	{
		mTreeView.getTreeView().sortByColumn(0, Qt::SortOrder::AscendingOrder); 
	});
}
