/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "standarditemsproperty.h"
#include "widgetdelegate.h"
#include "menuoptioncontroller.h"
#include "thememanager.h"

#include <QStandardItemModel>
#include <QMenu>
#include <QLabel>

#include <rtti/object.h>
#include <napqt/filtertreeview.h>

namespace napkin
{
	/**
	 * The MIME type of a nap propertypath
	 */
	inline constexpr char* sNapkinMimeData = (char*)"application/napkin-path";

	class ArrayPropertyItem;


    /**
     * Data model backing the inspector panel tree view
     */
	class InspectorModel : public QStandardItemModel
	{
		Q_OBJECT
	public:
		/**
		 * This is a constructor
		 */
		InspectorModel();

		/**
		 * Set the path to edit
		 * @param path The object or property to edit
		 */
		void setPath(const PropertyPath& path);

		/**
		 * Retrieve the path that is currently being edited
		 */
		 const PropertyPath& path() const;

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
		 * Clear the model of its items. Unlike clear(), it leaves the headers etc.
		 */
		void clearItems();

		/**
		 * Clear the property path that is edited, subsequently clears the model items as well
		 */
		void clearPath();

		/**
	 	 * Run through the object's properties and create items for them
		 */
		void populateItems();

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

	Q_SIGNALS:
		void childAdded(const QList<QStandardItem*> items);

	private:
		/**
		 * Check if a property is to be included in the inspector view
		 * @param prop The property to omit (or not)
		 * @return True when the property should not be displayed, false otherwise
		 */
		bool isPropertyIgnored(const PropertyPath& prop) const;

		/**
		 * Called when a child is inserted
		 * @param path path to property that is inserted
		 * @param items new row items
		 */
		void onChildAdded(QList<QStandardItem*> items);

		PropertyPath mPath; // the path currently being edited by the property editor
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
		 * Show this path in the inspector.
		 * @param object The object shown in the inspector.
		 */
		void setPath(const PropertyPath& path);

	private:
		/**
		 * Clear out the properties from this panel
		 */
		void clear();

		/**
		 * Lays out all the items
		 */
		void expandTree(const QModelIndex& parent);

		/**
		 * Called when the context menu for an item should be shown
		 * @param menu The menu that actions should be added to (initially empty)
		 */
		void onItemContextMenu(QMenu& menu);

		/**
		 * Called when another property needs to be selected
		 */
		void onPropertySelectionChanged(const PropertyPath& prop);

		/**
		 * Called when the property index changes
		 */
		void onIndexSwapped(const PropertyPath& prop, size_t fromIndex, size_t toIndex);

		/**
		 * Called when an object has been removed
		 */
	 	void onObjectRemoved(nap::rtti::Object* obj);

		/**
		 * Called when a child is inserted into the model
		 * @param path path to the newly inserted child
		 * @param items row items
		 */
		void onChildAdded(QList<QStandardItem*> items);
		
		/**
		 * Called just before the current document is closed
		 * @param filename The name of the document
		 */
		void onFileClosing(const QString& filename);

		/**
		 * Called when an object is renamed
		 */
		void onObjectRenamed(nap::rtti::Object& object, const std::string& oldName, const std::string& newName);

		/**
		 * Creates possible actions for menu hook
		 */
		void createMenuCallbacks();

		/**
		 * Occurs when theme changes
		 */
		void themeChanged(const Theme& theme);

	private:
		InspectorModel mModel;						// The model for the view
		nap::qt::FilterTreeView mTreeView;			// A tree view
		QVBoxLayout mLayout;						// The main layout
		PropertyValueItemDelegate mWidgetDelegate;	// Display a different editor based on the property type

		QHBoxLayout mHeaderLayout;					// Layout for top part (includes title and subtitle)
		QHBoxLayout mSubHeaderLayout;				// Just below the header
		QLabel mTitle;								// Title label
		QLabel mSubTitle;							// Subtitle label
		QLabel mPathLabel;							// label before path
		QLineEdit mPathField;						// Display path to object
		PropertyPath mPath;							// Path to display
		MenuOptionController<PropertyPathItem> mMenuController;	// Menu option controller
	};
};
