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
#include <color.h>
#include <mathutils.h>
#include <nap/assert.h>

RTTI_DEFINE_BASE(napkin::PropertyPathItem)
RTTI_DEFINE_BASE(napkin::PropertyItem)
RTTI_DEFINE_BASE(napkin::CompoundPropertyItem)
RTTI_DEFINE_BASE(napkin::ArrayPropertyItem)
RTTI_DEFINE_BASE(napkin::PointerItem)
RTTI_DEFINE_BASE(napkin::PointerValueItem)
RTTI_DEFINE_BASE(napkin::ColorValueItem)
RTTI_DEFINE_BASE(napkin::EmbeddedPointerItem)
RTTI_DEFINE_BASE(napkin::EmbeddedPointerValueItem)
RTTI_DEFINE_BASE(napkin::PropertyValueItem)

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
			items << new EmbeddedPointerValueItem(path);
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


napkin::PropertyPathItem::PropertyPathItem(const PropertyPath& path) : mPath(path)
{
	setText(QString::fromStdString(path.getName()));
	connect(&AppContext::get(), &AppContext::propertyValueChanged, this, &PropertyPathItem::onPropertyValueChanged);
	connect(&AppContext::get(), &AppContext::objectRenamed, this, &PropertyPathItem::onObjectRenamed);
	connect(&AppContext::get(), &AppContext::removingObject, this, &PropertyPathItem::onRemovingObject);
}


QVariant napkin::PropertyPathItem::data(int role) const
{
	if (role == Qt::DisplayRole)
	{
		// If the parent is an array, display the index of this item
		auto parent_path = qobject_cast<PropertyPathItem*>(parentItem());
		if (parent_path != nullptr && parent_path->getPath().isArray())
		{
			return row();
		}
	}
	return QStandardItem::data(role);
}


void napkin::PropertyPathItem::onPropertyValueChanged(const PropertyPath& path)
{
	if (this->mPath == path)
	{
		valueChanged();
	}
}


void napkin::PropertyPathItem::onObjectRenamed(const nap::rtti::Object& object, const std::string& oldName, const std::string& newName)
{
	mPath.updateObjectName(oldName, newName);
}


void napkin::PropertyPathItem::onRemovingObject(const nap::rtti::Object* object)
{
	if (mPath.getObject() == object)
	{
		QStandardItem* parent_item = QStandardItem::parent();
		this->model()->removeRow(this->row(), parent_item != nullptr ?
			parent_item->index() : QModelIndex());
	}
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


//////////////////////////////////////////////////////////////////////////
// ArrayPropertyItem
//////////////////////////////////////////////////////////////////////////

napkin::ArrayPropertyItem::ArrayPropertyItem(const PropertyPath& path)
	: PropertyPathItem(path)
{
	std::string pathStr = path.getPath().toString();
	populateChildren();
	connect(&AppContext::get(), &AppContext::propertyChildInserted, this, &ArrayPropertyItem::onChildInserted);
	connect(&AppContext::get(), &AppContext::propertyChildRemoved, this, &ArrayPropertyItem::onChildRemoved);
}


void napkin::ArrayPropertyItem::populateChildren()
{
	for (auto childPath : mPath.getChildren())
		appendRow(createPropertyItemRow(childPath));
}


void napkin::ArrayPropertyItem::onChildInserted(const PropertyPath& parentPath, size_t childIndex)
{
	if (mPath == parentPath)
	{
		// Delete everything at and beyond the item that was inserted
		NAP_ASSERT_MSG(childIndex == this->rowCount(), "Item not appended to end of array");
		if (childIndex < this->rowCount())
			this->removeRows(childIndex, this->rowCount() - 1);

		// Create items for children from that point onwards
		auto children = mPath.getChildren();
		auto idx = childIndex;
		for (auto idx = childIndex; idx < children.size(); idx++)
			appendRow(createPropertyItemRow(children[idx]));

		// Notify listeners
		QList<QStandardItem*> child_items;
		child_items.reserve(3);
		for (int c = 0; c < this->columnCount(); c++)
			child_items.append(this->child(childIndex, c));

		childInserted(children[childIndex], child_items);
	}
}


void napkin::ArrayPropertyItem::onChildRemoved(const PropertyPath& parentPath, size_t childIndex)
{
	if (mPath == parentPath)
	{
		// Every child item beyond the one that was deleted now maps to the wrong index.
		// Instead of deleting (and re-creating) all items beyond the item that was removed, update their paths
		assert(childIndex < rowCount());
		auto idx = childIndex + 1;
		while (idx < this->rowCount())
		{
			for (int c = 0; c < this->columnCount(); c++)
			{
				auto a = qitem_cast<PropertyPathItem*>(this->child(idx - 1, c));
				auto b = qitem_cast<PropertyPathItem*>(this->child(idx - 0, c));
				if (a != nullptr && b != nullptr)
				{
					b->setPath(a->getPath());
				}
			}
			idx++;
		}
		// Now remove the item
		this->removeRows(childIndex, 1);
	}
}


//////////////////////////////////////////////////////////////////////////
// PointerItem
//////////////////////////////////////////////////////////////////////////

napkin::PointerItem::PointerItem(const PropertyPath& path)
	: PropertyPathItem(path)
{ }


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
		// Get nap color
		nap::rtti::ResolvedPath resolved = mPath.resolve();
		assert(resolved.getType().is_derived_from(RTTI_OF(nap::BaseColor)));
		nap::rtti::Variant var = resolved.getValue();
		const nap::BaseColor& color = var.get_value<nap::BaseColor>();

		// Create color for display conversion
		nap::RGBAColor8 display_color;
		assert(color.getNumberOfChannels() <= display_color.getNumberOfChannels());

		// Create base 16 color display
		nap::BaseColor::Converter color_converter = color.getConverter(display_color);
		std::string display = "#";
		for (int i = 0; i < color.getNumberOfChannels(); i++)
		{
			color_converter(color, display_color, i);
			display += nap::utility::stringFormat("%02x", display_color[i]);
		}
		return QString::fromStdString(display).toUpper();
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
	// Ensure we can parse string, must be dividable by 2
	QString color_str = value.toString();
	color_str.remove('#');
	if (color_str.size() % 2 != 0)
		return;

	// Resolve path
	nap::rtti::ResolvedPath resolved = mPath.resolve();
	assert(resolved.getType().is_derived_from(RTTI_OF(nap::BaseColor)));
	nap::rtti::Variant var = resolved.getValue();
	const nap::BaseColor& target_color = var.get_value<nap::BaseColor>();

	// Number of channels to set: min of the two.
	// Allows for a wide range of edits, including single channel entries
	int channel_count = nap::math::min<int>(color_str.size() / 2, target_color.getNumberOfChannels());

	// Convert string to nap color RGBA8, which can hold max number of channels
	nap::RGBAColor8 nap_color;
	assert(channel_count <= nap_color.getNumberOfChannels());

	bool valid = false;
	for (int i = 0; i < channel_count; i++)
	{
		nap_color[i] = static_cast<nap::uint8>(color_str.midRef(i * 2, 2).toUInt(&valid, 16));
		if (!valid)
		{
			return;
		}
	}

	// Create target color using RTTI and set.
	// The color constructor, registered with RTTI, is called and converts the RGBA8 color for us.
	rttr::variant new_color = resolved.getType().create({ *static_cast<nap::BaseColor*>(&nap_color) });
	if (!new_color.is_valid())
	{
		assert(false);	///< Color could not be constructed!
		return;
	}
	resolved.setValue(new_color);
}


//////////////////////////////////////////////////////////////////////////
// EmbeddedPointerItem
//////////////////////////////////////////////////////////////////////////

static nap::rtti::Object* getEmbeddedObject(const nap::rtti::ResolvedPath& path)
{
	// First resolve the pointee, after that behave like compound
	assert(path.isValid());
	auto value = path.getValue();
	auto value_type = value.get_type();
	auto wrapped_type = value_type.is_wrapper() ? value_type.get_wrapped_type() : value_type;
	bool is_wrapper = wrapped_type != value_type;
	return  is_wrapper ?
		value.extract_wrapped_value().get_value<nap::rtti::Object*>() :
		value.get_value<nap::rtti::Object*>();
}


napkin::EmbeddedPointerItem::EmbeddedPointerItem(const PropertyPath& path)
	: PropertyPathItem(path)
{
	populateChildren();
	connect(this, &PropertyPathItem::valueChanged, this, &EmbeddedPointerItem::onValueChanged);
}


void napkin::EmbeddedPointerItem::populateChildren()
{
	// First resolve the pointee, after that behave like compound
	// If the embedded object isn't present, do nothing
	nap::rtti::Object* pointee = getEmbeddedObject(mPath.resolve());
	if (pointee == nullptr)
		return;

	auto object = pointee;
	for (auto childprop : object->get_type().get_properties())
	{
		auto childValue = childprop.get_value(object);
		std::string name = childprop.get_name().data();
		if(name == nap::rtti::sIDPropertyName)
			continue;

		nap::rtti::Path path;
		path.pushAttribute(name);
		appendRow(createPropertyItemRow({ *object, path, *(AppContext::get().getDocument())}));
	}
}


void napkin::EmbeddedPointerItem::onValueChanged()
{
	// Remove embedded child (all children) and re-populate
	auto* parent_item = QStandardItem::parent();
	removeRows(0, this->rowCount());
	populateChildren();
}


//////////////////////////////////////////////////////////////////////////
// EmbeddedPointerValueItem
//////////////////////////////////////////////////////////////////////////

napkin::EmbeddedPointerValueItem::EmbeddedPointerValueItem(const PropertyPath& path)
	: PropertyPathItem(path)
{
	setEditable(true);
}


QVariant napkin::EmbeddedPointerValueItem::data(int role) const
{
	switch (role)
	{
		case Qt::DisplayRole:
		case Qt::EditRole:
		{
			nap::rtti::Object* pointee = getEmbeddedObject(mPath.resolve());
			return pointee != nullptr ? pointee->mID.c_str() : "NULL";
		}
		default:
		{
			return PropertyPathItem::data(role);
		}
	}
}


void napkin::EmbeddedPointerValueItem::setData(const QVariant& value, int role)
{
	switch (role)
	{
		case Qt::EditRole:
		{
			nap::rtti::Object* pointee = getEmbeddedObject(mPath.resolve());
			if (pointee != nullptr)
			{
				PropertyPath id_path(pointee->mID, nap::rtti::sIDPropertyName, *mPath.getDocument());
				napkin::AppContext::get().executeCommand(new SetValueCommand(id_path, value));
			}
			break;
		}
		default:
		{
			PropertyPathItem::setData(value, role);
			break;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
// PropertyValueItem
//////////////////////////////////////////////////////////////////////////

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
{ }
