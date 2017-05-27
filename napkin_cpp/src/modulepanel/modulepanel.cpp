#include "modulepanel.h"


ModulePanel::ModulePanel(QWidget* parent)
	: QDockWidget(parent), onModuleLoadedSlot(this, &ModulePanel::onModuleLoaded)
{
	ui.setupUi(this);
	ui.treeView->setModel(&mModel);
}


void ModulePanel::onModuleLoaded(nap::Module& module) { refresh(); }

void ModulePanel::refresh()
{
	mModel.refresh();
	for (int row = 0; row < mModel.rowCount(); row++) {
		auto item = mModel.index(row, 0);
		ui.treeView->setExpanded(item, true);
	}
}
