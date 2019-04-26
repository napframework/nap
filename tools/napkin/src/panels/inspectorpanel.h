#pragma once

#include <QStandardItemModel>
#include <QMenu>
#include <QLabel>

#include <rtti/object.h>

#include "propertypath.h"
#include <napqt/filtertreeview.h>
#include "widgetdelegate.h"

namespace napkin
{
	/**
	 * The MIME type of a nap propertypath
	 */
	static const char* sNapkinMimeData = "application/napkin-path";

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
		void setObject(nap::rtti::Object* object);

		/**
		 * @return The object currently displayed/edited by this model
		 */
		nap::rtti::Object* getObject();

		/**
		 * http://doc.qt.io/qt-4.8/qabstractitemmodel.html#data
		 */
		QVariant data(const QModelIndex& index, int role) const override;

		/**
		 * http://doc.qt.io/qt-4.8/qabstractitemmodel.html#setData
		 */
		bool setData(const QModelIndex& index, const QVariant& value, int role) override;

		/**
		 * Override, provides drag behavior with validation and data to hold in the drag operation
		 */
		QMimeData* mimeData(const QModelIndexList& indexes) const override;

		/**
		 * Override, specifies which kinds of data will be provided to drag operations
		 */
		QStringList mimeTypes() const override;

		/**
		 * Rebuild the model
		 */
		void rebuild();

		/**
		 * Overriden to support drag & drop
		 */
		Qt::DropActions supportedDragActions() const override;

		/**
		 * Overriden to support drag & drop
		 */
		Qt::DropActions supportedDropActions() const override;

		/**
		 * Overriden to support drag & drop
		 */
		Qt::ItemFlags flags(const QModelIndex& index) const override;

	private:
		/**
		 * Check if a property is to be included in the inspector view
		 * @param prop The property to omit (or not)
		 * @return True when the property should not be displayed, false otherwise
		 */
		bool isPropertyIgnored(const PropertyPath& prop) const;
		/**
		 * Run through the object's properties and create items for them
		 */
		void populateItems();

		nap::rtti::Object* mObject = nullptr; // The object currently used by this model
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
		void setObject(nap::rtti::Object* object);

	private:
		/**
		 * Called when the context menu for an item should be shown
		 * @param menu The menu that actions should be added to (initially empty)
		 */
		void onItemContextMenu(QMenu& menu);

		/**
		 * Called when a property value has been changed
		 */
		void onPropertyValueChanged(const PropertyPath& path);

		/**
		 * Called when another property needs to be selected
		 */
		void onPropertySelectionChanged(const PropertyPath& prop);

		/**
		 * Rebuild view and model
		 */
		void rebuild();

	private:
		InspectorModel mModel;					   // The model for the view
		nap::qt::FilterTreeView mTreeView;	       // A tree view
		QVBoxLayout mLayout;					   // The main layout
		PropertyValueItemDelegate mWidgetDelegate; // Display a different editor based on the property type

		QHBoxLayout mHeaderLayout;				   // Layout for top part (includes title and subtitle)
		QLabel mTitle;							   // Title label
		QLabel mSubTitle;                          // Subtitle label
	};
};