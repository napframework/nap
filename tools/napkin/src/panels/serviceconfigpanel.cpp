/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "serviceconfigpanel.h"

#include <napkin-resources.h>
#include <appcontext.h>

namespace napkin
{
	//////////////////////////////////////////////////////////////////////////
	// Item
	//////////////////////////////////////////////////////////////////////////

	ServiceConfigItem::ServiceConfigItem(nap::ServiceConfiguration& config, Document& document)
		: QStandardItem(), mConfig(config), mDocument(&document)
	{
		std::string service_type = nap::utility::stripNamespace(config.getServiceType().get_name().to_string());
		setText(QString::fromStdString(service_type));
		setEditable(false);
	}


	napkin::PropertyPath ServiceConfigItem::propertyPath()
	{
		return PropertyPath(mConfig, *mDocument);
	}


	QVariant ServiceConfigItem::data(int role) const
	{
		switch (role)
		{
		case Qt::DecorationRole:
			return AppContext::get().getResourceFactory().getIcon(QRC_ICONS_CONFIGURATION);
		default:
			return QStandardItem::data(role);
		}
	}


	//////////////////////////////////////////////////////////////////////////
	// Model
	//////////////////////////////////////////////////////////////////////////

	ServiceConfigModel::ServiceConfigModel() : QStandardItemModel()
	{
		auto ctx = &AppContext::get();
		connect(ctx, &AppContext::coreInitialized, this, &ServiceConfigModel::populate);
		connect(ctx, &AppContext::serviceConfigurationChanged, this, &ServiceConfigModel::populate);
		connect(ctx, &AppContext::serviceConfigurationClosing, this, &ServiceConfigModel::onClosing);
		setHorizontalHeaderLabels({ "Service" });
	}


	void ServiceConfigModel::populate()
	{
		// Clear and create new set of items
		auto& ctx = AppContext::get();
		assert(ctx.hasServiceConfig());
		const auto& config_list = ctx.getServiceConfig()->getList();
		for (const auto& config : config_list)
		{
			auto item = new ServiceConfigItem(*config, ctx.getServiceConfig()->getDocument());
			appendRow(item);
		}
		populated();
	}


	void ServiceConfigModel::onClosing(QString file)
	{
		clearItems();
	}


	void ServiceConfigModel::clearItems()
	{
		removeRows(0, rowCount());
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
		mTreeView.enableSorting();

		// Listen to changes
		connect(mTreeView.getSelectionModel(), &QItemSelectionModel::selectionChanged, this, &ServiceConfigPanel::onSelectionChanged);
		connect(&mModel, &ServiceConfigModel::populated, this, [&]()
		{
			mTreeView.getTreeView().sortByColumn(0, Qt::SortOrder::AscendingOrder);
		});
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
