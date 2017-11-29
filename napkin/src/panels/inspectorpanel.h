#pragma once

#include "generic/napgeneric.h"
#include "generic/utility.h"
#include "widgetdelegate.h"
#include <QList>
#include <QStandardItemModel>
#include <QWidget>
#include <generic/customdelegate.h>
#include <generic/filtertreeview.h>
#include <nap/logger.h>
#include <nap/objectptr.h>
#include <rtti/rttiobject.h>
#include <rtti/rttipath.h>

namespace napkin
{
	enum InspectorPanelStandardItemTypeID
	{
		EmptyItemTypeID = 10,
		InvalidItemTypeID = 11,
		BaseItemTypeID = 12,
		PropertyItemTypeID = 13,
		PropertyValueItemTypeID = 14,
		EmbeddedPointerItemTypeID = 15,
		CompoundPropertyItemTypeID = 16,
		ArrayPropertyItemTypeID = 17,
		PointerItemTypeID = 18,
		PointerValueItemTypeID = 19,
	};

	/**
	 * Create a row of items where each item sits in its own column
	 * @param type The type of the item
	 * @param name The name of the item
	 * @param object The object owning the property this item row represents
	 * @param path The path to the property this row represents
	 * @param prop The property this item row represents
	 * @param value The value of the property in this row
	 * @return a row of items where each item sits in its own column
	 */
	QList<QStandardItem*> createItemRow(rttr::type type, const QString& name, nap::rtti::RTTIObject* object,
										const nap::rtti::RTTIPath& path, rttr::property prop, rttr::variant value);

		/**
		 * An empty item.
		 */
		class EmptyItem : public QStandardItem
	{
	public:
		/**
		 * QStandardItem is not a QObject, so regular QObject polymorphism doesn't work.
		 * This function is supposed to solve that.
		 * See: http://doc.qt.io/qt-5/qstandarditem.html#type
		 */
		int type() const override;

		/**
		 * Constructor
		 */
		EmptyItem();
	};

	/**
	 * An item representing invalid data
	 */
	class InvalidItem : public QStandardItem
	{
	public:
		/**
		 * QStandardItem is not a QObject, so regular QObject polymorphism doesn't work.
		 * This function is supposed to solve that.
		 * See: http://doc.qt.io/qt-5/qstandarditem.html#type
		 */
		int type() const override;

		/**
		 * @param name Text to be displayed
		 */
		explicit InvalidItem(const QString& name);
	};

	/**
	 * The base for items that represent a nap::rtti::RTTIObject
	 */
	class BaseItem : public QStandardItem
	{
	public:
		/**
		 * QStandardItem is not a QObject, so regular QObject polymorphism doesn't work.
		 * This function is supposed to solve that.
		 * See: http://doc.qt.io/qt-5/qstandarditem.html#type
		 */
		int type() const override;

		/**
		 * @param name Text on the item.
		 * @param object The object to keep track of.
		 * @param path The path to the property on the object
		 */
		BaseItem(const QString& name, nap::rtti::RTTIObject* object, const nap::rtti::RTTIPath& path);

	protected:
		/**
		 * Resolve a path to get a resolved path.
		 * @return A resolved path
		 */
		nap::rtti::ResolvedRTTIPath resolvePath();

		nap::rtti::RTTIObject* mObject;  // The object we're keeping track of
		const nap::rtti::RTTIPath mPath; // The path to the property on the object
	};

	/**
	 * This item shows the name of an object's property
	 */
	class PropertyItem : public BaseItem
	{
	public:
		/**
		 * QStandardItem is not a QObject, so regular QObject polymorphism doesn't work.
		 * This function is supposed to solve that.
		 * See: http://doc.qt.io/qt-5/qstandarditem.html#type
		 */
		int type() const override;

		/**
		 * @param name Text to display.
		 * @param object The object to keep track of.
		 * @param path The path to the property.
		 */
		PropertyItem(const QString& name, nap::rtti::RTTIObject* object, const nap::rtti::RTTIPath& path);
	};

	/**
	 * This property is has child properties
	 */
	class CompoundPropertyItem : public BaseItem
	{
	public:
		/**
		 * QStandardItem is not a QObject, so regular QObject polymorphism doesn't work.
		 * This function is supposed to solve that.
		 * See: http://doc.qt.io/qt-5/qstandarditem.html#type
		 */
		int type() const override;

		/**
		 * @param name Text to display.
		 * @param object The object to keep track of.
		 * @param path The path to the property.
		 */
		CompoundPropertyItem(const QString& name, nap::rtti::RTTIObject* object, const nap::rtti::RTTIPath& path);

	private:
		void populateChildren();
	};

	/**
	 * The property is an editable list of child properties
	 */
	class ArrayPropertyItem : public BaseItem
	{
	public:
		/**
		 * QStandardItem is not a QObject, so regular QObject polymorphism doesn't work.
		 * This function is supposed to solve that.
		 * See: http://doc.qt.io/qt-5/qstandarditem.html#type
		 */
		int type() const override;

		/**
		 * @param name Text to display.
		 * @param object The object to keep track of.
		 * @param path The path to the property.
		 * @param prop The property the path is pointing to.
		 * @param array Because the property is an array, provide a view into the array.
		 */
		ArrayPropertyItem(const QString& name, nap::rtti::RTTIObject* object, const nap::rtti::RTTIPath& path,
						  rttr::property prop, rttr::variant_array_view array);

	private:
		void populateChildren();

		rttr::property mProperty;
		rttr::variant_array_view mArray;
	};

	/**
	 * This item shows
	 */
	class PointerItem : public BaseItem
	{
	public:
		/**
		 * QStandardItem is not a QObject, so regular QObject polymorphism doesn't work.
		 * This function is supposed to solve that.
		 * See: http://doc.qt.io/qt-5/qstandarditem.html#type
		 */
		int type() const override;

		/**
		 * @param name Text to display.
		 * @param object The object to keep track of.
		 * @param path The path to the property, pointer.
		 */
		PointerItem(const QString& name, nap::rtti::RTTIObject* object, const nap::rtti::RTTIPath path);
	};


	class PointerValueItem : public QStandardItem
	{
	public:
		/**
		 * QStandardItem is not a QObject, so regular QObject polymorphism doesn't work.
		 * This function is supposed to solve that.
		 * See: http://doc.qt.io/qt-5/qstandarditem.html#type
		 */
		int type() const override;

		/**
		 * @param name Text to display.
		 * @param object The object to keep track of.
		 * @param path The path to the property, pointer.
		 */
		PointerValueItem(nap::rtti::RTTIObject* object, const nap::rtti::RTTIPath path, rttr::type valueType);

		/**
		 * Reimplemented from QStandardItem
		 */
		QVariant data(int role) const override;

		/**
		 * Reimplemented from QStandardItem
		 */
		void setData(const QVariant& value, int role) override;

		/**
		 * @return The type of the value represented by the pointer.
		 */
		rttr::type valueType();

	private:
		nap::rtti::RTTIObject* mObject; // The object to keep track of
		nap::rtti::RTTIPath mPath;		// The path to the property
		rttr::type mValueType;			// The type of the value represented by the pointer
	};

	/**
	 * Creates children, data under the embedded pointer
	 */
	class EmbeddedPointerItem : public BaseItem
	{
	public:
		/**
		 * QStandardItem is not a QObject, so regular QObject polymorphism doesn't work.
		 * This function is supposed to solve that.
		 * See: http://doc.qt.io/qt-5/qstandarditem.html#type
		 */
		int type() const override;

		/**
		 * @param name Text to display.
		 * @param object The object to keep track of.
		 * @param path The path to the property, pointer.
		 */
		EmbeddedPointerItem(const QString& name, nap::rtti::RTTIObject* object, nap::rtti::RTTIPath path);

	private:
		/**
		 * Populate child items
		 */
		void populateChildren();
	};

	/**
	 * This item displays the value of an object property and allows the user to change it
	 */
	class PropertyValueItem : public BaseItem
	{
	public:
		/**
		 * QStandardItem is not a QObject, so regular QObject polymorphism doesn't work.
		 * This function is supposed to solve that.
		 * See: http://doc.qt.io/qt-5/qstandarditem.html#type
		 */
		int type() const override;

		/**
		 * @param name Text to display.
		 * @param object The object to keep track of.
		 * @param path The path to the property, pointer.
		 * @param valueType The type of the value
		 */
		PropertyValueItem(const QString& name, nap::rtti::RTTIObject* object, nap::rtti::RTTIPath path,
						  rttr::type valueType);

		/**
		 * Reimplemented from QStandardItem
		 */
		QVariant data(int role) const override;

		/**
		 * Reimplemented from QStandardItem
		 */
		void setData(const QVariant& value, int role) override;

		/**
		 * @return The type of the value held by the property.
		 */
		rttr::type& valueType();

	private:
		rttr::type mValueType; // The type of the value held by the property.
	};

	/**
	 * Data model backing the inspector panel tree view
	 */
	class InspectorModel : public QStandardItemModel
	{
	public:
		/**
		 * This is a constructor
		 */
		InspectorModel();

		/**
		 * Set the
		 * @param object
		 */
		void setObject(nap::rtti::RTTIObject* object);

		/**
		 * @return The object currently displayed/edited by this model
		 */
		nap::rtti::RTTIObject* object();

		/**
		 * http://doc.qt.io/qt-4.8/qabstractitemmodel.html#data
		 */
		QVariant data(const QModelIndex& index, int role) const override;

		/**
		 * http://doc.qt.io/qt-4.8/qabstractitemmodel.html#setData
		 */
		bool setData(const QModelIndex& index, const QVariant& value, int role) override;

	private:
		/**
		 * Run through the object's properties and create items for them
		 */
		void populateItems();

		nap::rtti::RTTIObject* mObject = nullptr; // The object currently used by this model
	};

	/**
	 * The inspector panel allows for inspection and changing of object properties using a tree view
	 */
	class InspectorPanel : public QWidget
	{
		Q_OBJECT
	public:
		/**
		 * Constructor!
		 */
		InspectorPanel();

		/**
		 * Show this object in the inspector.
		 * @param object The object shown in the inspector.
		 */
		void setObject(nap::rtti::RTTIObject* object);


	private:
		InspectorModel mModel;					   // The model for the view
		FilterTreeView mTreeView;				   // A tree view
		QVBoxLayout mLayout;					   // The main layout
		PropertyValueItemDelegate mWidgetDelegate; // Display a different editor based on the property type
	};
};