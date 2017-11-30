#include "standarditemsgeneric.h"
#include "napkinglobals.h"


napkin::EmptyItem::EmptyItem() : QStandardItem()
{
	setEditable(false);
}

int napkin::EmptyItem::type() const
{
	return QStandardItem::UserType + StandardItemTypeID::EmptyID;
}

napkin::InvalidItem::InvalidItem(const QString& name) : QStandardItem(name)
{
	setForeground(Qt::red);
	setEditable(false);
}

int napkin::InvalidItem::type() const
{
	return QStandardItem::UserType + StandardItemTypeID::InvalidID;
}
