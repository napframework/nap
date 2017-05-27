#include "modulemodel.h"

ModuleManagerModel::ModuleManagerModel() : QStandardItemModel()
{
	AppContext::get().core().getModuleManager().moduleLoaded.connect([&](nap::Module& module) { refresh(); });
}



void ModuleManagerModel::refresh()
{
	clear();
	const QIcon& folderIcon = *AppContext::get().iconStore().get("folder");
	const QIcon& convertIcon = *AppContext::get().iconStore().get("convert");

	for (nap::Module* module : AppContext::get().core().getModuleManager().getModules()) {
		auto moduleItem = new ModuleItem(*module);
		moduleItem->setIcon(*AppContext::get().iconStore().iconFor(RTTI_OF(nap::Module)));
		appendRow(moduleItem);

		auto convertersItem = new ModuleBaseItem(folderIcon, "Type Converters");
		{
			moduleItem->appendRow(convertersItem);
			for (const nap::TypeConverterBase* conv : module->getTypeConverters()) {
				TypeConverterItem* item = new TypeConverterItem(conv);
				item->setIcon(convertIcon);
				convertersItem->appendRow(item);
			}
		}

		nap::TypeList typeList;
		module->getDataTypes(typeList);

		auto dataTypesItem = new ModuleBaseItem(folderIcon, "Data Types");
		{
			moduleItem->appendRow(dataTypesItem);
			for (const auto& dataType : typeList) {
				QString name = QString("%1 (%2)").arg(QString::fromStdString(dataType.getName()),
													  QString::number(dataType.getId()));
				dataTypesItem->appendRow(new QStandardItem(name));
			}
		}

		typeList.clear();
		module->getComponentTypes(typeList);

		auto componentTypesItem = new ModuleBaseItem(folderIcon, "Component Types");
		{
			moduleItem->appendRow(componentTypesItem);
			for (const auto& componentType : typeList) {
				QString name = QString("%1 (%2)").arg(QString::fromStdString(componentType.getName()),
													  QString::number(componentType.getId()));
				componentTypesItem->appendRow(new QStandardItem(name));
			}
		}

		typeList.clear();
		module->getOperatorTypes(typeList);

		auto operatorTypesItem = new ModuleBaseItem(folderIcon, "Operator Types");
		{
			moduleItem->appendRow(operatorTypesItem);
			for (const auto& operatorType : typeList) {
				QString name = QString("%1 (%2)").arg(QString::fromStdString(operatorType.getName()),
													  QString::number(operatorType.getId()));
				operatorTypesItem->appendRow(new QStandardItem(name));
			}
		}
	}
}
