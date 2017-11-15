#pragma once

#include <QStandardItemModel>

#include <nap/core.h>
#include "objectitem.h"

class OutlineModel : public QStandardItemModel
{
public:
	void setCore(nap::Core* core);

private:
	nap::Core* mCore;
};


