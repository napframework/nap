#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <napqt/filtertreeview.h>

namespace napkin
{

	class ModuleItem : public QStandardItem
	{
	public:
		ModuleItem(const nap::ModuleManager::Module& module);

	private:
		const nap::ModuleManager::Module& mModule;
	};

	class ModuleModel : public QStandardItemModel
	{
	public:
		ModuleModel();
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
