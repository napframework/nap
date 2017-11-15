#include "outlinemodel.h"

#include "objectitem.h"

void OutlineModel::setCore(nap::Core* core)
{
	clear();
	mCore = core;
    auto rootItem = new ObjectItem(mCore->getRoot());
	appendRow(rootItem);
}


