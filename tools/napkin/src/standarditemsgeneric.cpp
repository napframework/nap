#include "standarditemsgeneric.h"
#include "napkinglobals.h"


napkin::EmptyItem::EmptyItem() : QStandardItem()
{
	setEditable(false);
}

napkin::InvalidItem::InvalidItem(const QString& name) : QStandardItem(name)
{
	setForeground(Qt::red);
	setEditable(false);
}
