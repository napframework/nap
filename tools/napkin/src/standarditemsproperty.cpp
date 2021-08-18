/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "commands.h"
#include "napkinglobals.h"
#include "naputils.h"
#include "standarditemsproperty.h"
#include "standarditemsgeneric.h"
#include "appcontext.h"

#include <QtDebug>
#include <rtti/object.h>

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
	else if (path.isColor())
	{
		items << new CompoundPropertyItem(path);
		items << new ColorValueItem(path);
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

}

QVariant napkin::PropertyPathItem::data(int role) const
{
	if (role == Qt::DisplayRole)
	{
		// If the parent is an array, display the index of this item
		if (auto parentPath = dynamic_cast<PropertyPathItem*>(parent()))
		{
			if (parentPath->getPath().isArray())
				return row();
		}
	}
	return QStandardItem::data(role);
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
		if (auto pointee = mPath.getPointee())
			return QString::fromStdString(pointee->mID);
		else
			return "NULL";
	}
	else if (role == Qt::UserRole)
	{
		return QVariant::fromValue(mPath);
	}
	if (mPath.isInstanceProperty() && mPath.isOverridden() && role == Qt::BackgroundRole)
		return QStandardItem::data(role);
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
	: PropertyPathItem(path)
{ }


napkin::ColorValueItem::ColorValueItem(const PropertyPath& path) : PropertyPathItem(path)
{ }


QVariant napkin::ColorValueItem::data(int role) const
{
	switch (role)
	{
	case Qt::DisplayRole:
	case Qt::EditRole:
	{
		return "";
	}
	case Qt::UserRole:
	{
		return QVariant::fromValue(mPath);
	}
	default:
		return QStandardItem::data(role);
	}
}


void napkin::ColorValueItem::setData(const QVariant& value, int role)
{
	nap::Logger::info("Set Color!");
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
		nap::Logger::warn("Embedded pointer was null: %s", mPath.toString().c_str());
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

		appendRow(createPropertyItemRow({ *object, path, *(AppContext::get().getDocument())}));
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
