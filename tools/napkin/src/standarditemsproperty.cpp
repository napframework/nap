#include <rtti/object.h>
#include "commands.h"

#include "napkinglobals.h"
#include "naputils.h"
#include "standarditemsproperty.h"
#include "standarditemsgeneric.h"
#include "appcontext.h"

#include <QtDebug>

QList<QStandardItem*> napkin::createPropertyItemRow(const PropertyPath& path)
{
	QList<QStandardItem*> items;

	auto type = path.getType();

	if (path.isArray())
	{
		items << new ArrayPropertyItem(path);
		items << new EmptyItem();
		items << new RTTITypeItem(type);
	}
	else if (type.is_associative_container())
	{
        // Probably not supported anyway
		assert(false);
	}
	else if (path.isPointer())
	{
		if (path.isEmbeddedPointer())
		{
			items << new EmbeddedPointerItem(path);
			items << new EmptyItem();
			items << new RTTITypeItem(type);
		}
		else
		{
			items << new PointerItem(path);
			items << new PointerValueItem(path);
			items << new RTTITypeItem(type);
		}
	}
	else if (nap::rtti::isPrimitive(type))
	{
		items << new PropertyItem(path);
		items << new PropertyValueItem(path);
		items << new RTTITypeItem(type);
	}
	else
	{
        // Assuming leftovers are compounds
		items << new CompoundPropertyItem(path);
		items << new EmptyItem();
		items << new RTTITypeItem(type);
	}
	return items;
}



napkin::PropertyPathItem::PropertyPathItem(const PropertyPath& path)
	: QStandardItem(QString::fromStdString(path.getName())), mPath(path)
{
//	auto txt = text();
//	nap::Logger::info(txt.toStdString());
//	setText(txt);
}

napkin::PropertyItem::PropertyItem(const PropertyPath& path)
	: PropertyPathItem(path)
{
	setEditable(false);
}

void napkin::CompoundPropertyItem::populateChildren()
{
	for (auto childPath : mPath.getChildren())
		appendRow(createPropertyItemRow(childPath));
}

napkin::CompoundPropertyItem::CompoundPropertyItem(const PropertyPath& path)
	: PropertyPathItem(path)
{
	populateChildren();
}

void napkin::ArrayPropertyItem::populateChildren()
{
	for (auto childPath : mPath.getChildren())
		appendRow(createPropertyItemRow(childPath));
}

napkin::ArrayPropertyItem::ArrayPropertyItem(const PropertyPath& path)
	: PropertyPathItem(path)
{
	std::string pathStr = path.getPath().toString();
	populateChildren();
}

napkin::PointerItem::PointerItem(const PropertyPath& path)
	: PropertyPathItem(path)
{
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
		napkin::AppContext::get().executeCommand(new SetPointerValueCommand(mPath, new_target));
	} 
	else 
	{
		QStandardItem::setData(value, role);
	}
}

napkin::PointerValueItem::PointerValueItem(const PropertyPath& path)
	: QStandardItem(), mPath(path)
{
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

		auto wrappedType = childValue.get_type().is_wrapper() ? childValue.get_type().get_wrapped_type() : childValue.get_type();


		appendRow(createPropertyItemRow({*object, path}));
	}
}

napkin::EmbeddedPointerItem::EmbeddedPointerItem(const PropertyPath& path)
	: PropertyPathItem(path)
{
	populateChildren();
}

QVariant napkin::PropertyValueItem::data(int role) const
{

	if (role == Qt::DisplayRole || role == Qt::EditRole)
	{
		QVariant variant;
		if (napkin::toQVariant(mPath.getType(), mPath.getValue(), variant))
			return variant;

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
		auto path = mPath;
		napkin::AppContext::get().executeCommand(new SetValueCommand(path, value));
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

napkin::PropertyValueItem::PropertyValueItem(const PropertyPath& path)
	: PropertyPathItem(path)
{
}

