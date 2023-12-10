/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "propertypath.h"
#include "rttiitem.h"

#include <QStandardItem>
#include <QVBoxLayout>
#include <rtti/object.h>
#include <rtti/path.h>
#include <rtti/rtti.h>

namespace napkin
{
	/**
	 * Create a row of items where each item sits in its own column
	 * @param path The path to the property this row represents
	 * @return a row of items where each item sits in its own column
	 */
	QList<QStandardItem*> createPropertyItemRow(const PropertyPath& path);


	/**
	 * The base for items that represent a property path, RTTI enabled
	 */
	class PropertyPathItem : public RTTIItem
	{
		Q_OBJECT
	public:
		/**
		 * @param name Text on the item.
		 * @param object The object to keep track of.
		 * @param path The path to the property on the object
		 */
		PropertyPathItem(const PropertyPath& path);

		/**
		 * Override from QStandardItem
		 */
		QVariant data(int role) const override;

		/**
		 * The path held by this item
		 */
		const PropertyPath& getPath() const { return mPath; }

	Q_SIGNALS:
		/**
		 * Called when the property value of this item changes
		 */
		void valueChanged();

		/**
		 * Called when the object name changed.
		 * @param oldName the old (now invalid) object name
		 * @param newName the new (now valid) object name
		 */
		void objectRenamed(const std::string& oldName, PropertyPath newName);

		/**
		 * Called when a new row is added to this item.
		 * @param items row items
		 */
		void childAdded(const QList<QStandardItem*> items);

	protected:
		PropertyPath mPath; // The path to the property

		/**
		 * Creates and appends a new row to this item based on the given child path
		 * Call this in derived classes to create a set of child items for a child path.
		 * @param childPath property child path
		 * @return items that were added
		 */
		QList<QStandardItem*> createAppendRow(const PropertyPath& childPath);

	private:
		// Called when a property value changes
		void onPropertyValueChanged(const PropertyPath& path);

		// Called when an object name changes
		void onObjectRenamed(const nap::rtti::Object& object, const std::string& oldName, const std::string& newName);

		// Called just before an object is removed
		void onRemovingObject(const nap::rtti::Object* object);
	};


	/**
	 * This item shows the name of an object's property
	 */
	class PropertyItem : public PropertyPathItem
	{
		Q_OBJECT
	public:
		/**
		 * @param name Text to display.
		 * @param object The object to keep track of.
		 * @param path The path to the property.
		 */
		PropertyItem(const PropertyPath& path);
	};


	/**
	 * This property is has child properties
	 */
	class CompoundPropertyItem : public PropertyPathItem
	{
		Q_OBJECT
	public:
		/**
		 * @param name Text to display.
		 * @param object The object to keep track of.
		 * @param path The path to the property.
		 */
		CompoundPropertyItem(const PropertyPath& path);

	private:
        /**
         * Generate child items
         */
		void populateChildren();
	};


	/**
	 * The property is an editable list of child properties
	 */
	class ArrayPropertyItem : public PropertyPathItem
	{
		Q_OBJECT
	public:
		/**
		 * @param name Text to display.
		 * @param object The object to keep track of.
		 * @param path The path to the property.
		 * @param prop The property the path is pointing to.
		 * @param array Because the property is an array, provide a view into the array.
		 */
		ArrayPropertyItem(const PropertyPath& path);

		/**
		 * Override from QStandardItem
		 */
		QVariant data(int role) const override;

	private:
        /**
         * Generate child items
         */
		void populateChildren();

		/**
		 * Called when a child item is inserted
		 */
		void onChildInserted(const PropertyPath& parentPath, size_t childIndex);

		/**
		 * Called when a child item is removed
		 */
		void onChildRemoved(const PropertyPath& parentPath, size_t childIndex);
	};


	/**
	 * This item shows an object pointer
	 */
	class PointerItem : public PropertyPathItem
	{
		Q_OBJECT
	public:
		/**
		 * @param name Text to display.
		 * @param object The object to keep track of.
		 * @param path The path to the property, pointer.
		 */
		PointerItem(const PropertyPath& path);
	};


    /**
     * This item allows for editing a pointer value
     */
	class PointerValueItem : public PropertyPathItem
	{
		Q_OBJECT
	public:
		/**
		 * @param name Text to display.
		 * @param object The object to keep track of.
		 * @param path The path to the property, pointer.
		 */
		PointerValueItem(const PropertyPath& path);

		/**
		 * Reimplemented from QStandardItem
		 */
		QVariant data(int role) const override;

		/**
		 * Reimplemented from QStandardItem
		 */
		void setData(const QVariant& value, int role) override;
	};


	/**
	 * This item allows for editing a color value
	 */
	class ColorValueItem : public PropertyPathItem
	{
		Q_OBJECT
	public:
		/**
		 * @param path The path to the property, pointer.
		 */
		ColorValueItem(const PropertyPath& path);

		/**
		 * Reimplemented from QStandardItem
		 */
		QVariant data(int role) const override;

		/**
		 * Reimplemented from QStandardItem
		 */
		void setData(const QVariant& value, int role) override;
	};


	/**
	 * Creates children, data under the embedded pointer
	 */
	class EmbeddedPointerItem : public PropertyPathItem
	{
		Q_OBJECT
	public:
		/**
		 * @param path The path to the embedded pointer property
		 */
		EmbeddedPointerItem(const PropertyPath& path);

	private:
		/**
		 * Populate child items
		 */
		void populateChildren();

		// Called when this property value changes
		void onValueChanged();
	};


	/**
	 * Embedded object pointer value
	 */
	class EmbeddedPointerValueItem : public PropertyPathItem
	{
	public:
		/**
		 * @param path The path to the embedded pointer property
		 */
		EmbeddedPointerValueItem(const PropertyPath& path);

	private:
		QVariant data(int role) const override;
		void setData(const QVariant& value, int role) override;
	};


	/**
	 * This item displays the value of an object property and allows the user to change it
	 */
	class PropertyValueItem : public PropertyPathItem
	{
		Q_OBJECT
	public:
		/**
		 * @param name Text to display.
		 * @param object The object to keep track of.
		 * @param path The path to the property, pointer.
		 * @param valueType The type of the value
		 */
		PropertyValueItem(const PropertyPath& path);

		/**
		 * Reimplemented from QStandardItem
		 */
		QVariant data(int role) const override;

		/**
		 * Reimplemented from QStandardItem
		 */
		void setData(const QVariant& value, int role) override;
	};
}; // napkin
