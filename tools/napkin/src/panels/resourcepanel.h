#pragma once

#include <standarditemsobject.h>
#include <napqt/filtertreeview.h>
#include "actions.h"


namespace nap
{
	namespace rtti
	{
		class Object;
	}
}


namespace napkin
{


	/**
	 * Model containing full list of resources in the system. Hierarchy is represented where possible.
	 * The data is retrieved through AppContext
	 */
	class ResourceModel : public QStandardItemModel
	{
	public:
		ResourceModel();

		/**
		 * Clear all the items from the model and rebuild
		 */
		void refresh();

		/**
		 * Clears all items from the model
		 */
		void clear();

		/**
		 * Add an item (row) to represent an Object
		 * @param object the object to represent
		 */
		ObjectItem* addObjectItem(nap::rtti::Object& object);

		/**
		 * Remove an item (row) representing an Object
		 * @param object remove the item that represents this Object
		 */
		void removeObjectItem(const nap::rtti::Object& object);

		/**
		 * Find all Objects that are pointed to by an embedded pointer and remove the corresponding items
		 */
		void removeEmbeddedObjects();

	private:
		GroupItem mObjectsItem; // top level item that will hold objects/resources
		GroupItem mEntitiesItem; // top level item that will hold entities
	};

	/**
	 * A panel showing all the loaded resources in the system.
	 */
	class ResourcePanel : public QWidget
	{
		Q_OBJECT
	public:
		ResourcePanel();

		/**
		 * Set the current selection of the treeview to the given objects
		 */
		void selectObjects(const QList<nap::rtti::Object*>& selection);

		/**
		 * @return The tree view held by this panel
		 */
		nap::qt::FilterTreeView& treeView() { return mTreeView; }

	Q_SIGNALS:
		void selectionChanged(QList<PropertyPath> obj);

	private:
		/**
		 * Reconstruct the list
		 */
		void refresh();

		/**
		 * Called when an entity has been added
		 * @param newEntity The entity that was added
		 * @param parent The parent of the entity that was added
		 */
		void onEntityAdded(nap::Entity* newEntity, nap::Entity* parent);

		/**
		 * Called when a component has been added
		 * @param comp The component that was added
		 * @param owner The owner of the component
		 */
		void onComponentAdded(nap::Component* comp, nap::Entity* owner);

		/**
		 * Called when an object has been added
		 * @param obj The object that was added
		 * @param selectNewObject Whether the newly created object should be selected in any views watching for object addition
		 */
		void onObjectAdded(nap::rtti::Object* obj, bool selectNewObject);

		/**
		 * Called when an object is about to be removed
		 * @param obj The object that will be removed
		 */
		void onObjectRemoved(const nap::rtti::Object* obj);

		/**
		 * Called when a new file was created.
		 */
		void onNewFile();

		/**
		 * Called just after a file has been opened
		 * @param filename The name of the file that was opened
		 */
		void onFileOpened(const QString& filename);

		/**
		 * Called just after a file has been closed
		 * @param filename The name of the file that was just closed
		 */
		void onFileClosed(const QString& filename);

		/**
		 * Called when the global selection was changed
		 * @param selected The items that were selected
		 * @param deselected The items that were deselected
		 */
		void onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

		/**
		 * Called when an object name has changed
		 */
		void onPropertyValueChanged(const PropertyPath& path);

		/**
		 * Used to provide this view with custom menu items
		 * @param menu The menu to append items to.
		 */
		void menuHook(QMenu& menu);

	private:
		QVBoxLayout mLayout;	  // Layout
		ResourceModel mModel;	 // Model
		nap::qt::FilterTreeView mTreeView; // Treeview
	};
}