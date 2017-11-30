#include "commands.h"
#include "napkinglobals.h"
#include <generic/utility.h>
#include "propertyitems.h"
#include "genericitems.h"


QList<QStandardItem*> napkin::createPropertyItemRow(rttr::type type, const QString& name, nap::rtti::RTTIObject* object,
													const nap::rtti::RTTIPath& path, rttr::property prop,
													rttr::variant value)
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



napkin::BaseItem::BaseItem(const QString& name, nap::rtti::RTTIObject* object, const nap::rtti::RTTIPath& path)
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
	return UserType + InspectorPanelStandardItemTypeID::BaseItemTypeID;
}

napkin::PropertyItem::PropertyItem(const QString& name, nap::rtti::RTTIObject* object, const nap::rtti::RTTIPath& path)
	: BaseItem(name, object, path)
{
	setEditable(false);
	setForeground(napkin::getSoftForeground());
}

int napkin::PropertyItem::type() const
{
	return UserType + InspectorPanelStandardItemTypeID::PropertyItemTypeID;
}

void napkin::CompoundPropertyItem::populateChildren()
{
	auto resolved = resolvePath();
	auto compound = resolved.getValue();

	for (auto childprop : compound.get_type().get_properties())
	{
		auto value = childprop.get_value(compound);
		std::__cxx11::string name = childprop.get_name().data();
		QString qName = QString::fromStdString(name);

		nap::rtti::RTTIPath path = mPath;
		path.pushAttribute(name);
		auto wrappedType = value.get_type().is_wrapper() ? value.get_type().get_wrapped_type() : value.get_type();

		appendRow(createPropertyItemRow(wrappedType, qName, mObject, path, childprop, value));
	}
}

napkin::CompoundPropertyItem::CompoundPropertyItem(const QString& name, nap::rtti::RTTIObject* object,
												   const nap::rtti::RTTIPath& path)
	: BaseItem(name, object, path)
{
	setForeground(napkin::getSoftForeground());
	populateChildren();
}

int napkin::CompoundPropertyItem::type() const
{
	return UserType + InspectorPanelStandardItemTypeID::CompoundPropertyItemTypeID;
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

		appendRow(createPropertyItemRow(wrappedType, name, mObject, path, mProperty, value));
	}
}

napkin::ArrayPropertyItem::ArrayPropertyItem(const QString& name, nap::rtti::RTTIObject* object,
											 const nap::rtti::RTTIPath& path, rttr::property prop,
											 rttr::variant_array_view array)
	: BaseItem(name, object, path), mProperty(prop), mArray(array)
{
	std::__cxx11::string pathStr = path.toString();
	populateChildren();
	setForeground(napkin::getSoftForeground());
}

int napkin::ArrayPropertyItem::type() const
{
	return UserType + InspectorPanelStandardItemTypeID::ArrayPropertyItemTypeID;
}

napkin::PointerItem::PointerItem(const QString& name, nap::rtti::RTTIObject* object, const nap::rtti::RTTIPath path)
	: BaseItem(name, object, path)
{
	setForeground(napkin::getSoftForeground());
}

int napkin::PointerItem::type() const
{
	return UserType + InspectorPanelStandardItemTypeID::PointerItemTypeID;
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

napkin::PointerValueItem::PointerValueItem(nap::rtti::RTTIObject* object, const nap::rtti::RTTIPath path,
										   rttr::type valueType)
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
	return UserType + InspectorPanelStandardItemTypeID::PointerValueItemTypeID;
}

void napkin::PointerValueItem::setData(const QVariant& value, int role)
{
	QStandardItem::setData(value, role);
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
		std::__cxx11::string name = childprop.get_name().data();
		QString qName = QString::fromStdString(name);

		nap::rtti::RTTIPath path;
		path.pushAttribute(name);

		auto wrappedType =
			childValue.get_type().is_wrapper() ? childValue.get_type().get_wrapped_type() : childValue.get_type();


		appendRow(createPropertyItemRow(wrappedType, qName, object, path, childprop, childValue));
	}
}

napkin::EmbeddedPointerItem::EmbeddedPointerItem(const QString& name, nap::rtti::RTTIObject* object,
												 nap::rtti::RTTIPath path)
	: BaseItem(name, object, path)
{
	populateChildren();
}

int napkin::EmbeddedPointerItem::type() const
{
	return UserType + InspectorPanelStandardItemTypeID::EmbeddedPointerItemTypeID;
}

QVariant napkin::PropertyValueItem::data(int role) const
{

	if (role == Qt::DisplayRole || role == Qt::EditRole)
	{
		nap::rtti::ResolvedRTTIPath resolvedPath;
		assert(mPath.resolve(mObject, resolvedPath));
		QVariant variant;
		if (napkin::toQVariant(resolvedPath.getType(), resolvedPath.getValue(), variant))
		{
			return variant;
		}

		return napkin::TXT_UNCONVERTIBLE_TYPE;
	}
	return QStandardItem::data(role);
}

void napkin::PropertyValueItem::setData(const QVariant& value, int role)
{
	if (role == Qt::EditRole)
	{
		auto undoCommand = new napkin::SetValueCommand(mObject, mPath, value);
		napkin::AppContext::get().getUndoStack().push(undoCommand);
	}

	if (role == Qt::DisplayRole)
	{
		nap::rtti::ResolvedRTTIPath resolvedPath;
		assert(mPath.resolve(mObject, resolvedPath));
		bool ok;
		auto resultValue = napkin::fromQVariant(resolvedPath.getType(), value, &ok);
		if (ok)
			resolvedPath.setValue(resultValue);
	}
	QStandardItem::setData(value, role);
}

napkin::PropertyValueItem::PropertyValueItem(const QString& name, nap::rtti::RTTIObject* object,
											 nap::rtti::RTTIPath path, rttr::type valueType)
	: BaseItem(name, object, path), mValueType(valueType)
{
}

int napkin::PropertyValueItem::type() const
{
	return UserType + InspectorPanelStandardItemTypeID::PropertyValueItemTypeID;
}

rttr::type& napkin::PropertyValueItem::getValueType()
{
	return mValueType;
}