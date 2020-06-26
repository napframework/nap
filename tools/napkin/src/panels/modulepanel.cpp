#include <appcontext.h>
#include "modulepanel.h"

using namespace napkin;

ModuleItem::ModuleItem(const nap::Module& module)
		: QStandardItem(), mModule(module)
{
	std::string name(module.mDescriptor->mID);
	setText(QString::fromStdString(name));
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
	auto core = AppContext::get().getCore();
	if (!core)
		return;

	for (const auto& mod : core->getModuleManager().getModules())
		appendRow(new ModuleItem(*mod));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ModulePanel::ModulePanel() : QWidget()
{
	setLayout(&mLayout);
	mLayout.setContentsMargins(0, 0, 0, 0);
	mLayout.addWidget(&mTreeView);
	mTreeView.setModel(&mModel);
}
