/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <napqt/filtertreeview.h>
#include <nap/modulemanager.h>

namespace napkin
{

	class ModuleItem : public QStandardItem
	{
	public:
		ModuleItem(const nap::Module& module);

	private:
		const nap::Module& mModule;
	};

	class ModuleModel : public QStandardItemModel
	{
		Q_OBJECT
	public:
		ModuleModel();

	Q_SIGNALS:
		void populated();

	private:
		void onCoreInitialized();
	};

	class ModulePanel : public QWidget
	{
	public:
		ModulePanel();

	private:
		QVBoxLayout mLayout;
		nap::qt::FilterTreeView mTreeView;
		ModuleModel mModel;
	};
}
