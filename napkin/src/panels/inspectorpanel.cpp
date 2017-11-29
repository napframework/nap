#include "inspectorpanel.h"
#include "commands.h"
#include "napkinglobals.h"
#include "typeconversion.h"
#include <QtWidgets/QApplication>
#include <generic/utility.h>

using namespace nap::rtti;

QList<QStandardItem*> napkin::createItemRow(rttr::type type, const QString& name, nap::rtti::RTTIObject* object,
                                            const nap::rtti::RTTIPath& path, rttr::property prop, rttr::variant value)
{
    QList<QStandardItem*> items;
    if (type.is_array())
    {
        items << new ArrayPropertyItem(name, object, path, prop, value.create_array_view());
        items << new EmptyItem();
        items << new RTTITypeItem(type);
    }
    else if (type.is_associative_container())
    {
        assert(false);
    }
    else if (type.is_pointer())
    {
        if (nap::rtti::hasFlag(prop, nap::rtti::EPropertyMetaData::Embedded))
        {
            items << new EmbeddedPointerItem(name, object, path);
            items << new EmptyItem();
            items << new RTTITypeItem(type);
        }
        else
        {
            items << new PointerItem(name, object, path);
            items << new PointerValueItem(object, path, type);
            items << new RTTITypeItem(type);
        }
    }
    else if (nap::rtti::isPrimitive(type))
    {
        items << new PropertyItem(name, object, path);
        items << new PropertyValueItem(name, object, path, type);
        items << new RTTITypeItem(type);
    }
    else
    {
        items << new CompoundPropertyItem(name, object, path);
        items << new EmptyItem();
        items << new RTTITypeItem(prop.get_type());
    }
    return items;
}


napkin::EmptyItem::EmptyItem() : QStandardItem()
{
	setEditable(false);
}

int napkin::EmptyItem::type() const
{
	return QStandardItem::UserType + InspectorPanelStandardItemTypeID::EmptyItemTypeID;
}

napkin::InvalidItem::InvalidItem(const QString& name) : QStandardItem(name)
{
	setForeground(Qt::red);
	setEditable(false);
}

int napkin::InvalidItem::type() const
{
	return QStandardItem::UserType + InspectorPanelStandardItemTypeID::InvalidItemTypeID;
}

napkin::BaseItem::BaseItem(const QString& name, nap::rtti::RTTIObject* object, const RTTIPath& path)
	: QStandardItem(name), mObject(object), mPath(path)
{
	nap::rtti::ResolvedRTTIPath resolved;
	assert(path.resolve(object, resolved));
	assert(mObject);
}

nap::rtti::ResolvedRTTIPath napkin::BaseItem::resolvePath()
{
	nap::rtti::ResolvedRTTIPath resolvedPath;
	assert(mPath.resolve(mObject, resolvedPath));
	return resolvedPath;
}

int napkin::BaseItem::type() const
{
	return QStandardItem::UserType + InspectorPanelStandardItemTypeID::BaseItemTypeID;
}

napkin::PropertyItem::PropertyItem(const QString& name, nap::rtti::RTTIObject* object, const nap::rtti::RTTIPath& path)
	: napkin::BaseItem(name, object, path)
{
	setEditable(false);
	setForeground(getSoftForeground());
}

int napkin::PropertyItem::type() const
{
	return QStandardItem::UserType + InspectorPanelStandardItemTypeID::PropertyItemTypeID;
}


QVariant napkin::PropertyValueItem::data(int role) const
{

	if (role == Qt::DisplayRole || role == Qt::EditRole)
	{
		nap::rtti::ResolvedRTTIPath resolvedPath;
		assert(mPath.resolve(mObject, resolvedPath));
		QVariant variant;
		if (toQVariant(resolvedPath.getType(), resolvedPath.getValue(), variant))
		{
			return variant;
		}

		return TXT_UNCONVERTIBLE_TYPE;
	}
	return QStandardItem::data(role);
}

void napkin::PropertyValueItem::setData(const QVariant& value, int role)
{
	if (role == Qt::EditRole)
	{
		auto undoCommand = new SetValueCommand(mObject, mPath, value);
        AppContext::get().getUndoStack().push(undoCommand);
	}

	if (role == Qt::DisplayRole)
	{
		nap::rtti::ResolvedRTTIPath resolvedPath;
		assert(mPath.resolve(mObject, resolvedPath));
		bool ok;
		auto resultValue = fromQVariant(resolvedPath.getType(), value, &ok);
		if (ok)
			resolvedPath.setValue(resultValue);
	}
	QStandardItem::setData(value, role);
}

napkin::PropertyValueItem::PropertyValueItem(const QString& name, nap::rtti::RTTIObject* object, nap::rtti::RTTIPath path,
									 rttr::type valueType)
	: BaseItem(name, object, path), mValueType(valueType)
{
}

int napkin::PropertyValueItem::type() const
{
	return QStandardItem::UserType + InspectorPanelStandardItemTypeID::PropertyValueItemTypeID;
}

rttr::type& napkin::PropertyValueItem::getValueType()
{
	return mValueType;
}


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


void napkin::ArrayPropertyItem::populateChildren()
{
	auto array = mArray;

	for (int i = 0; i < array.get_size(); i++)
	{

		auto name = QString("%1").arg(i);

		nap::rtti::RTTIPath path = mPath;
		path.pushArrayElement(i);

		auto value = array.get_value(i);
		auto type = array.get_rank_type(array.get_rank());
		auto wrappedType = type.is_wrapper() ? type.get_wrapped_type() : value.get_type();

		appendRow(createItemRow(wrappedType, name, mObject, path, mProperty, value));
	}
}

napkin::ArrayPropertyItem::ArrayPropertyItem(const QString& name, nap::rtti::RTTIObject* object,
									 const nap::rtti::RTTIPath& path, rttr::property prop,
									 rttr::variant_array_view array)
	: BaseItem(name, object, path), mProperty(prop), mArray(array)
{
	std::string pathStr = path.toString();
	populateChildren();
	setForeground(getSoftForeground());
}

int napkin::ArrayPropertyItem::type() const
{
	return QStandardItem::UserType + InspectorPanelStandardItemTypeID::ArrayPropertyItemTypeID;
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

		appendRow(createItemRow(wrappedType, qName, mObject, path, prop, value));
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

void napkin::CompoundPropertyItem::populateChildren()
{
	auto resolved = resolvePath();
	auto compound = resolved.getValue();

	for (auto childprop : compound.get_type().get_properties())
	{
		auto value = childprop.get_value(compound);
		std::string name = childprop.get_name().data();
		QString qName = QString::fromStdString(name);

		nap::rtti::RTTIPath path = mPath;
		path.pushAttribute(name);
		auto wrappedType = value.get_type().is_wrapper() ? value.get_type().get_wrapped_type() : value.get_type();

		appendRow(createItemRow(wrappedType, qName, mObject, path, childprop, value));
	}
}

napkin::CompoundPropertyItem::CompoundPropertyItem(const QString& name, nap::rtti::RTTIObject* object,
										   const nap::rtti::RTTIPath& path)
	: napkin::BaseItem(name, object, path)
{
	setForeground(getSoftForeground());
	populateChildren();
}

int napkin::CompoundPropertyItem::type() const
{
	return QStandardItem::UserType + InspectorPanelStandardItemTypeID::CompoundPropertyItemTypeID;
}

napkin::PointerItem::PointerItem(const QString& name, nap::rtti::RTTIObject* object, const nap::rtti::RTTIPath path)
	: napkin::BaseItem(name, object, path)
{
	setForeground(getSoftForeground());
}

int napkin::PointerItem::type() const
{
	return QStandardItem::UserType + InspectorPanelStandardItemTypeID::PointerItemTypeID;
}

void napkin::EmbeddedPointerItem::populateChildren()
{
	// First resolve the pointee, after that behave like compound
	nap::rtti::ResolvedRTTIPath resolvedPath;
	assert(mPath.resolve(mObject, resolvedPath));
	auto value = resolvedPath.getValue();

	auto value_type = value.get_type();
	auto wrapped_type = value_type.is_wrapper() ? value_type.get_wrapped_type() : value_type;
	bool is_wrapper = wrapped_type != value_type;
	nap::rtti::RTTIObject* pointee = is_wrapper ? value.extract_wrapped_value().get_value<nap::rtti::RTTIObject*>()
												: value.get_value<nap::rtti::RTTIObject*>();
	if (nullptr == pointee)
	{
		assert(false); // Embedded pointer always has a target?
		return;
	}


	auto object = pointee;

	for (auto childprop : object->get_type().get_properties())
	{
		auto childValue = childprop.get_value(object);
		std::string name = childprop.get_name().data();
		QString qName = QString::fromStdString(name);

		nap::rtti::RTTIPath path;
		path.pushAttribute(name);

		auto wrappedType =
			childValue.get_type().is_wrapper() ? childValue.get_type().get_wrapped_type() : childValue.get_type();


		appendRow(createItemRow(wrappedType, qName, object, path, childprop, childValue));
	}
}

napkin::EmbeddedPointerItem::EmbeddedPointerItem(const QString& name, nap::rtti::RTTIObject* object, nap::rtti::RTTIPath path)
	: napkin::BaseItem(name, object, path)
{
	populateChildren();
}

int napkin::EmbeddedPointerItem::type() const
{
	return QStandardItem::UserType + InspectorPanelStandardItemTypeID::EmbeddedPointerItemTypeID;
}


QVariant napkin::PointerValueItem::data(int role) const
{
	if (role == Qt::DisplayRole || role == Qt::EditRole)
	{
		nap::rtti::ResolvedRTTIPath resolvedPath;
		assert(mPath.resolve(mObject, resolvedPath));
		auto value = resolvedPath.getValue();

		auto value_type = value.get_type();
		auto wrapped_type = value_type.is_wrapper() ? value_type.get_wrapped_type() : value_type;
		bool is_wrapper = wrapped_type != value_type;
		nap::rtti::RTTIObject* pointee = is_wrapper ? value.extract_wrapped_value().get_value<nap::rtti::RTTIObject*>()
													: value.get_value<nap::rtti::RTTIObject*>();

		if (nullptr != pointee)
			return QString::fromStdString(pointee->mID);
		else
			return "NULL";
	}
	return QStandardItem::data(role);
}

napkin::PointerValueItem::PointerValueItem(nap::rtti::RTTIObject* object, const nap::rtti::RTTIPath path, rttr::type valueType)
	: QStandardItem(), mObject(object), mPath(path), mValueType(valueType)
{
	setForeground(Qt::darkCyan);
	nap::rtti::ResolvedRTTIPath resolved;
	assert(path.resolve(object, resolved));
}

rttr::type napkin::PointerValueItem::getValueType()
{
	return mValueType;
}

int napkin::PointerValueItem::type() const
{
	return QStandardItem::UserType + InspectorPanelStandardItemTypeID::PointerValueItemTypeID;
}

void napkin::PointerValueItem::setData(const QVariant& value, int role)
{
	QStandardItem::setData(value, role);
}
