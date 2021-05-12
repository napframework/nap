/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "serviceconfigpanel.h"
#include <appcontext.h>

namespace napkin
{
	//////////////////////////////////////////////////////////////////////////
	// Item
	//////////////////////////////////////////////////////////////////////////

	ServiceConfigItem::ServiceConfigItem(nap::ServiceConfiguration& config, Document& document)
		: QStandardItem(), mConfig(config), mDocument(&document)
	{
		std::string service_type = config.getServiceType().get_name().to_string();
		setText(QString::fromStdString(service_type));
	}


	napkin::PropertyPath ServiceConfigItem::propertyPath()
	{
		return PropertyPath(mConfig, *mDocument);
	}


	//////////////////////////////////////////////////////////////////////////
	// Model
	//////////////////////////////////////////////////////////////////////////

	ServiceConfigModel::ServiceConfigModel() : QStandardItemModel()
	{
		auto ctx = &AppContext::get();
		connect(ctx, &AppContext::coreInitialized, this, &ServiceConfigModel::onCoreInitialized);
		setHorizontalHeaderLabels({ "Name" });
	}


	void ServiceConfigModel::onCoreInitialized()
	{
		removeRows(0, rowCount());		
		auto& ctx = AppContext::get();
		assert(ctx.hasServiceConfig());
		const auto& config_list = ctx.getServiceConfig()->getList();

		for (const auto& item : config_list)
		{
			appendRow(new ServiceConfigItem(*item, ctx.getServiceConfig()->getDocument()));
		}
	}


	//////////////////////////////////////////////////////////////////////////
	// Widget
	//////////////////////////////////////////////////////////////////////////

	ServiceConfigPanel::ServiceConfigPanel() : QWidget()
	{
		setLayout(&mLayout);
		mLayout.setContentsMargins(0, 0, 0, 0);
		mLayout.addWidget(&mTreeView);
		mTreeView.setModel(&mModel);

		// Listen to changes
		connect(mTreeView.getSelectionModel(), &QItemSelectionModel::selectionChanged, this,
			&ServiceConfigPanel::onSelectionChanged);
	}


	void ServiceConfigPanel::onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
	{
		QList<PropertyPath> paths;
		for (auto m : mTreeView.getSelectedItems())
		{
			auto item = dynamic_cast<ServiceConfigItem*>(m);
			assert(item != nullptr);
			paths << item->propertyPath();
		}
		selectionChanged(paths);
	}
}
