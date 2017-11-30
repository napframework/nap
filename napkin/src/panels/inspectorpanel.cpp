#include "inspectorpanel.h"
#include "commands.h"
#include "napkinglobals.h"
#include "typeconversion.h"
#include <QtWidgets/QApplication>
#include <generic/utility.h>
#include <generic/propertyitems.h>

using namespace nap::rtti;


void napkin::InspectorModel::setObject(RTTIObject* object)
{
	while (rowCount() > 0)
		removeRow(0);

	mObject = object;

	if (nullptr != mObject)
		populateItems();
}


napkin::InspectorModel::InspectorModel() : QStandardItemModel()
{
	setHorizontalHeaderLabels({TXT_LABEL_NAME, TXT_LABEL_VALUE, TXT_LABEL_TYPE});
}


napkin::InspectorPanel::InspectorPanel()
{
	setLayout(&mLayout);
	layout()->setContentsMargins(0, 0, 0, 0);
	mLayout.addWidget(&mTreeView);
	mTreeView.setModel(&mModel);
    mTreeView.getTreeView().setColumnWidth(0, 250);
    mTreeView.getTreeView().setColumnWidth(1, 250);
    mTreeView.getTreeView().setItemDelegateForColumn(1, &mWidgetDelegate);
}

void napkin::InspectorPanel::setObject(RTTIObject* objects)
{
	mModel.setObject(objects);
    mTreeView.getTreeView().expandAll();
}


void napkin::InspectorModel::populateItems()
{
	for (auto prop : mObject->get_type().get_properties())
	{
		if (!prop.is_valid() || prop.is_static() || prop.get_name() == PROP_CHILDREN ||
			prop.get_name() == PROP_COMPONENTS)
			continue;

		std::string name = prop.get_name().data();
		QString qName = QString::fromStdString(name);

		nap::rtti::RTTIPath path;
		path.pushAttribute(name);

		auto value = prop.get_value(mObject);
		auto wrappedType = value.get_type().is_wrapper() ? value.get_type().get_wrapped_type() : value.get_type();

		appendRow(createPropertyItemRow(wrappedType, qName, mObject, path, prop, value));
	}
}

QVariant napkin::InspectorModel::data(const QModelIndex& index, int role) const
{
	if (role == Qt::UserRole)
	{
		auto valueItem = dynamic_cast<PropertyValueItem*>(itemFromIndex(index));
		if (valueItem)
		{
			return QVariant::fromValue(TypeWrapper(&valueItem->getValueType()));
		}
	}
	return QStandardItemModel::data(index, role);
}

bool napkin::InspectorModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
	if (role == Qt::EditRole)
	{
		auto valueItem = dynamic_cast<PropertyValueItem*>(itemFromIndex(index));
		valueItem->setData(value, Qt::EditRole);
		return true;
	}
	return QStandardItemModel::setData(index, value, role);
}

nap::rtti::RTTIObject* napkin::InspectorModel::getObject()
{
	return mObject;
}


