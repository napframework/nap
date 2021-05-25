/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <napqt/filtertreeview.h>
#include <nap/service.h>

#include "propertypath.h"

namespace napkin
{
	/**
	 * Wraps a NAP service configuration
	 */
	class ServiceConfigItem : public QStandardItem
	{
	public:
		ServiceConfigItem(nap::ServiceConfiguration& config, Document& document);

		/**
		 * @return item as property path, can be visualized & edited in inspector
		 */
		PropertyPath propertyPath();

	private:
		nap::ServiceConfiguration& mConfig;
		Document* mDocument;
	};


	/**
	 * Simple model that manages a set of service configuration items
	 */
	class ServiceConfigModel : public QStandardItemModel
	{
	public:
		ServiceConfigModel();
	private:
		void populate();
		void onClosing(QString file);
		void clearItems();
	};


	/**
	 * Widget that shows all the available service configurations
	 */
	class ServiceConfigPanel : public QWidget
	{
	Q_OBJECT
	public:
		ServiceConfigPanel();

	Q_SIGNALS:
		void selectionChanged(QList<PropertyPath> configs);

	private:
		void onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

	private:
		QVBoxLayout mLayout;
		nap::qt::FilterTreeView mTreeView;
		ServiceConfigModel mModel;
	};
}
