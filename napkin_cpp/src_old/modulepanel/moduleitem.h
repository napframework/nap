#pragma once

#include <QStandardItem>
#include <nap/modulemanager.h>
#include "nap/module.h"


class ModuleBaseItem : public QStandardItem {
public:
    ModuleBaseItem() : QStandardItem() {
        setEditable(false);
    }

    ModuleBaseItem(const QIcon& icon, const QString& text) : QStandardItem(icon, text) {
        setEditable(false);
    }
};

class ModuleItem : public ModuleBaseItem {
public:
    ModuleItem(const nap::Module& module) : ModuleBaseItem(), mModule(module) {
        setText(QString::fromStdString(module.getName()));
    }
private:
    const nap::Module& mModule;
};

class TypeConverterItem : public ModuleBaseItem {
public:
	TypeConverterItem(const nap::TypeConverterBase* converter ) : ModuleBaseItem(), mConverter(converter) {
		setText(QString("%1 -> %2").arg(QString::fromStdString(converter->inType().getName()),
													 QString::fromStdString(converter->outType().getName())));
	}

private:
	const nap::TypeConverterBase* mConverter;
};