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
	static constexpr char* sNapkinMimeData = "application/napkin-path";

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
		 * Set the path to edit
		 * @param path The object or property to edit
		 */
		void setPath(const PropertyPath& path);

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

	private:
		/**
		 * Check if a property is to be included in the inspector view
		 * @param prop The property to omit (or not)
		 * @return True when the property should not be displayed, false otherwise
		 */
		bool isPropertyIgnored(const PropertyPath& prop) const;

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
		 * Rebuilds view and model and applies path as selection
		 * This is a temp work-around to ensure selection remains valid
		 * @param selection the property path that should be selected after rebuilding the model
		 * @param verticalScrollPos the vertical scroll position of the widget before being refreshed
		 */
		void rebuild(PropertyPath selection);
		
		/**
		 * Called just after a file has been closed
		 * @param filename The name of the file that was just closed
		 */
		void onFileClosed(const QString& filename);

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
	};
};