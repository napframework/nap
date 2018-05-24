#include "commands.h"

#include "napkinglobals.h"
#include "generic/qtutils.h"
#include "generic/naputils.h"
#include "standarditemsproperty.h"
#include "standarditemsgeneric.h"
#include "appcontext.h"


QList<QStandardItem*> napkin::createPropertyItemRow(rttr::type type, const QString& name, const PropertyPath& path,
													rttr::property prop, rttr::variant value)
{
	QList<QStandardItem*> items;
	if (type.is_array())
	{
		items << new ArrayPropertyItem(name, path, prop, value.create_array_view());
		items << new EmptyItem();
		items << new RTTITypeItem(type);
	}
	else if (type.is_associative_container())
	{
        // Probably not supported anyway
		assert(false);
	}
	else if (type.is_pointer())
	{
		if (nap::rtti::hasFlag(prop, nap::rtti::EPropertyMetaData::Embedded))
		{
			items << new EmbeddedPointerItem(name, path);
			items << new EmptyItem();
			items << new RTTITypeItem(type);
		}
		else
		{
			items << new PointerItem(name, path);
			items << new PointerValueItem(path, type);
			items << new RTTITypeItem(type);
		}
	}
	else if (nap::rtti::isPrimitive(type))
	{
		items << new PropertyItem(name, path);
		items << new PropertyValueItem(name, path, type);
		items << new RTTITypeItem(type);
	}
	else
	{
        // Assuming leftovers are compounds
		items << new CompoundPropertyItem(name, path);
		items << new EmptyItem();
		items << new RTTITypeItem(prop.get_type());
	}
	return items;
}



napkin::PropertyPathItem::PropertyPathItem(const QString& name, const PropertyPath& path)
	: QStandardItem(name), mPath(path)
{
//	auto txt = text();
//	nap::Logger::info(txt.toStdString());
//	setText(txt);
}

int napkin::PropertyPathItem::type() const
{
	return UserType + StandardItemTypeID::RTTIPathID;
}

napkin::PropertyItem::PropertyItem(const QString& name, const PropertyPath& path)
	: PropertyPathItem(name, path)
{
	setEditable(false);
}

int napkin::PropertyItem::type() const
{
	return UserType + StandardItemTypeID::PropertyID;
}

void napkin::CompoundPropertyItem::populateChildren()
{
	auto resolved = mPath.resolve();
	auto compound = resolved.getValue();

	for (auto childprop : compound.get_type().get_properties())
	{
		auto value = childprop.get_value(compound);
		std::string name = childprop.get_name().data();
		QString qName = QString::fromStdString(name);

		auto wrappedType = value.get_type().is_wrapper() ? value.get_type().get_wrapped_type() : value.get_type();

		appendRow(createPropertyItemRow(wrappedType, qName, mPath.getChild(name), childprop, value));
	}
}

napkin::CompoundPropertyItem::CompoundPropertyItem(const QString& name, const PropertyPath& path)
	: PropertyPathItem(name, path)
{
	populateChildren();
}

int napkin::CompoundPropertyItem::type() const
{
	return UserType + StandardItemTypeID::CompoundPropertyID;
}

void napkin::ArrayPropertyItem::populateChildren()
{
	auto array = mArray;

	for (int i = 0; i < array.get_size(); i++)
	{

		auto name = QString("%1").arg(i);

		nap::rtti::Path path = mPath.getPath();
		path.pushArrayElement(i);

        auto property = mPath.resolve().getProperty();
		auto value = array.get_value(i);
		auto type = array.get_rank_type(array.get_rank());
		auto wrappedType = type.is_wrapper() ? type.get_wrapped_type() : value.get_type();

		appendRow(createPropertyItemRow(wrappedType, name, {mPath.getObject(), path}, property,
										value));
	}
}

napkin::ArrayPropertyItem::ArrayPropertyItem(const QString& name, const PropertyPath& path, rttr::property prop,
											 rttr::variant_array_view array)
	: PropertyPathItem(name, path), mProperty(prop), mArray(array)
{
	std::string pathStr = path.getPath().toString();
	populateChildren();
}

int napkin::ArrayPropertyItem::type() const
{
	return UserType + StandardItemTypeID::ArrayPropertyID;
}

napkin::PointerItem::PointerItem(const QString& name, const PropertyPath& path)
	: PropertyPathItem(name, path)
{
}

int napkin::PointerItem::type() const
{
	return UserType + StandardItemTypeID::PointerID;
}

QVariant napkin::PointerValueItem::data(int role) const
{
	if (role == Qt::DisplayRole || role == Qt::EditRole)
	{
		nap::rtti::Object* pointee = mPath.getPointee();

		if (nullptr != pointee)
			return QString::fromStdString(pointee->mID);
		else
			return "NULL";
	}
	else if (role == Qt::UserRole)
	{
		return QVariant::fromValue(mPath);
	}
	return QStandardItem::data(role);
}

void napkin::PointerValueItem::setData(const QVariant& value, int role)
{
	if (role == Qt::EditRole) 
	{
		nap::rtti::Object* new_target = AppContext::get().getDocument()->getObject(value.toString().toStdString());
		if (new_target == nullptr)
			return;

		napkin::AppContext::get().executeCommand(new SetPointerValueCommand(mPath, new_target));
	} 
	else 
	{
		QStandardItem::setData(value, role);
	}
}

napkin::PointerValueItem::PointerValueItem(const PropertyPath& path, rttr::type valueType)
	: QStandardItem(), mPath(path), mValueType(valueType)
{
}

int napkin::PointerValueItem::type() const
{
	return UserType + StandardItemTypeID::PointerValueID;
}

void napkin::EmbeddedPointerItem::populateChildren()
{
	// First resolve the pointee, after that behave like compound
	nap::rtti::ResolvedPath resolvedPath = mPath.resolve();

	assert(resolvedPath.isValid());
	auto value = resolvedPath.getValue();

	auto value_type = value.get_type();
	auto wrapped_type = value_type.is_wrapper() ? value_type.get_wrapped_type() : value_type;
	bool is_wrapper = wrapped_type != value_type;
	nap::rtti::Object* pointee = is_wrapper ? value.extract_wrapped_value().get_value<nap::rtti::Object*>()
												: value.get_value<nap::rtti::Object*>();
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

		nap::rtti::Path path;
		path.pushAttribute(name);

		auto wrappedType =
			childValue.get_type().is_wrapper() ? childValue.get_type().get_wrapped_type() : childValue.get_type();


		appendRow(createPropertyItemRow(wrappedType, qName, {*object, path}, childprop,
										childValue));
	}
}

napkin::EmbeddedPointerItem::EmbeddedPointerItem(const QString& name, const PropertyPath& path)
	: PropertyPathItem(name, path)
{
	populateChildren();
}

int napkin::EmbeddedPointerItem::type() const
{
	return UserType + StandardItemTypeID::EmbeddedPointerID;
}

QVariant napkin::PropertyValueItem::data(int role) const
{

	if (role == Qt::DisplayRole || role == Qt::EditRole)
	{
		nap::rtti::ResolvedPath resolvedPath = mPath.resolve();
		assert(resolvedPath.isValid());
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
	nap::rtti::ResolvedPath resolvedPath = mPath.resolve();
	assert(resolvedPath.isValid());

	if (role == Qt::EditRole)
	{
		napkin::AppContext::get().executeCommand(new SetValueCommand(mPath, value));
	}
	else if (role == Qt::DisplayRole)
	{
		bool ok;
		auto resultValue = napkin::fromQVariant(resolvedPath.getType(), value, &ok);
		if (ok)
			resolvedPath.setValue(resultValue);
	}
	else
	{
		QStandardItem::setData(value, role);
	}
}

napkin::PropertyValueItem::PropertyValueItem(const QString& name, const PropertyPath& path, rttr::type valueType)
	: PropertyPathItem(name, path), mValueType(valueType)
{
}

int napkin::PropertyValueItem::type() const
{
	return UserType + StandardItemTypeID::PropertyValueID;
}

rttr::type& napkin::PropertyValueItem::getValueType()
{
	return mValueType;
}