#include "objectitems.h"
#include "napkinglobals.h"
#include <generic/utility.h>

napkin::GroupItem::GroupItem(const QString& name) : QStandardItem(name)
{
}

int napkin::GroupItem::type() const
{
	return QStandardItem::UserType + ResourcePanelPanelStandardItemTypeID::GroupItemTypeID;
}

napkin::ObjectItem::ObjectItem(nap::rtti::RTTIObject& rttiObject) : mObject(rttiObject)
{
	refresh();
}

void napkin::ObjectItem::refresh()
{
	QStandardItem::setText(getName());
}

const QString napkin::ObjectItem::getName() const
{
	return QString::fromStdString(mObject.mID);
}

int napkin::ObjectItem::type() const
{
	return QStandardItem::UserType + ResourcePanelPanelStandardItemTypeID::ObjectItemTypeID;
}

nap::rtti::RTTIObject& napkin::ObjectItem::getObject() const
{
	return mObject;
}

napkin::EntityItem::EntityItem(nap::Entity& entity) : ObjectItem(entity)
{

	for (auto& child : entity.mChildren)
	{
		appendRow({new EntityItem(*child), new napkin::RTTITypeItem(child->get_type())});
	}

	for (auto& comp : entity.mComponents)
	{
		auto compItem = new ComponentItem(*comp);
		auto compTypeItem = new napkin::RTTITypeItem(comp->get_type());
		appendRow({compItem, compTypeItem});
	}
}

int napkin::EntityItem::type() const
{
	return QStandardItem::UserType + ResourcePanelPanelStandardItemTypeID::EntityItemTypeID;
}

nap::Entity& napkin::EntityItem::getEntity()
{
	auto& e = *rtti_cast<nap::Entity*>(&mObject);
	return *e;
}

napkin::ComponentItem::ComponentItem(nap::Component& comp) : ObjectItem(comp)
{
}

int napkin::ComponentItem::type() const
{
	return QStandardItem::UserType + ResourcePanelPanelStandardItemTypeID::ComponentItemTypeID;
}

nap::Component& napkin::ComponentItem::getComponent()
{
	auto& o = *rtti_cast<nap::Component*>(&mObject);
	return *o;
}