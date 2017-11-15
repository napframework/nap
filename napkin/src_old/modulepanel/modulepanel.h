#pragma once

#include "modulemodel.h"
#include <QDockWidget>
#include <QStandardItemModel>
#include <QTreeView>
#include <nap/logger.h>
#include <ui_modulepanel.h>

namespace Ui
{
	class ModulePanel;
}

class ModulePanel : public QDockWidget
{
	Q_OBJECT
public:
	explicit ModulePanel(QWidget* parent = 0);

protected:
	virtual void showEvent(QShowEvent* event) override { refresh(); }


private:
	void refresh();

	void onModuleLoaded(nap::Module& module);
	nap::Slot<nap::Module&> onModuleLoadedSlot;

	Ui::ModulePanel ui;
	ModuleManagerModel mModel;
};
