#include "genericitems.h"


napkin::EmptyItem::EmptyItem() : QStandardItem()
{
	setEditable(false);
}

int napkin::EmptyItem::type() const
{
	return QStandardItem::UserType + GenericStandardItemTypeID::EmptyItemTypeID;
}

napkin::InvalidItem::InvalidItem(const QString& name) : QStandardItem(name)
{
	setForeground(Qt::red);
	setEditable(false);
}

int napkin::InvalidItem::type() const
{
	return QStandardItem::UserType + GenericStandardItemTypeID::InvalidItemTypeID;
}
