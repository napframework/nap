#pragma once

#include <QtGui/QStandardItem>
#include <QtWidgets/QVBoxLayout>
#include <generic/filtertreeview.h>
#include <rtti/rttiobject.h>
#include <rtti/rttipath.h>
#include <rttr/type.h>
#include <widgetdelegate.h>

namespace napkin
{


	class ArrayPropertyItem;

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
		nap::rtti::RTTIObject* getObject();

		/**
		 * http://doc.qt.io/qt-4.8/qabstractitemmodel.html#data
		 */
		QVariant data(const QModelIndex& index, int role) const override;

		/**
		 * http://doc.qt.io/qt-4.8/qabstractitemmodel.html#setData
		 */
		bool setData(const QModelIndex& index, const QVariant& value, int role) override;

		/**
		 * Rebuild the model
		 */
		void rebuild();

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
		/**
		 * Called when the context menu for an item should be shown
		 * @param menu The menu that actions should be added to (initially empty)
		 */
		void onItemContextMenu(QMenu& menu);

		/**
		 * Called when a property value has been changed
		 */
		void onPropertyValueChanged(nap::rtti::RTTIObject& object, const nap::rtti::RTTIPath& path);

		/**
		 * Add object to array represented by targetItem
		 * @param targetItem The item representing the array to add the object to
		 * @param object The object to add
		 */
		void onAddObjectArrayElement(ArrayPropertyItem* targetItem, nap::rtti::RTTIObject* object);

		/**
		 * Add object to array represented by targetItem
		 * @param targetItem The item representing the array to add the object to
		 * @param type The type of object to add. Object will be created internally.
		 */
		void onAddObjectArrayElement(ArrayPropertyItem* targetItem, const nap::rtti::TypeInfo& type);

		/**
		 * Rebuild view and model
		 */
		void rebuild();

	private:
		InspectorModel mModel;					   // The model for the view
		FilterTreeView mTreeView;				   // A tree view
		QVBoxLayout mLayout;					   // The main layout
		PropertyValueItemDelegate mWidgetDelegate; // Display a different editor based on the property type
	};
};