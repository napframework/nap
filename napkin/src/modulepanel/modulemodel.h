#pragma once

#include <nap/module.h>

#include "moduleitem.h"
#include "../appcontext.h"

class ModuleManagerModel : public QStandardItemModel
{
	Q_OBJECT
public:
	explicit ModuleManagerModel();
    void refresh();

private:
	nap::Core* mCore = nullptr;
};
