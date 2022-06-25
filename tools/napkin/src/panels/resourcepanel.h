/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "actions.h"
#include <standarditemsobject.h>
#include <napqt/filtertreeview.h>

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
		Q_OBJECT
	public:
		ResourceModel();

		/**
		 * Populates the entire model
		 */
		void populate();

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

	Q_SIGNALS:
		/**
		 * Triggered when a new child is added to an existing item
		 */
		void childAddedToGroup(GroupItem& group, ObjectItem& item);

	private:
		RegularResourcesItem mObjectsItem;		// top level item that will hold objects/resources
		EntityResourcesItem  mEntitiesItem;		// top level item that will hold entities
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
		 * Reconstruct the list, present items are destroyed before being rebuild.
		 * Internally the model is cleared and populated.
		 */
		void refresh();

		/**
		 * Clears the current model and enforces selection to be removed
		 */
		void clear();

		/**
		 * Populates the current model
		 */
		void populate();

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
		 * @param parent the parent of the object, nullptr if top level object
		 * @param selectNewObject Whether the newly created object should be selected in any views watching for object addition
		 */
		void onObjectAdded(nap::rtti::Object* obj, nap::rtti::Object* parent, bool selectNewObject);

		/**
		 * Called when an object is about to be removed
		 * @param obj The object that will be removed
		 */
		void onObjectRemoved(const nap::rtti::Object* obj);

		/**
		 * Called when an object moved to another group
		 * @param object the object that moved
		 * @param oldParent old parent property
		 * @param newParent new parent property
		 */
		void onObjectReparented(nap::rtti::Object& object, PropertyPath oldParent, PropertyPath newParent);

		/**
		 * Called just before an objects moves to a new group
		 * @param object the object that is going to be moved
		 * @param oldParent old parent property
		 * @param newParent new parent property
		 */
		void onObjectReparenting(nap::rtti::Object& object, PropertyPath currentParent, PropertyPath newParent);

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
		* Called just before the current document is closed
		 * @param filename the name of the document
		 */
		void onFileClosing(const QString& filename);

		/**
		 * Called when the global selection was changed
		 * @param selected The items that were selected
		 * @param deselected The items that were deselected
		 */
		void onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

		/**
		 * Called when a property value changed
		 */
		void onPropertyValueChanged(const PropertyPath& path);

		/**
		 * Called when a new item is inserted into an array
		 */
		void onChildAddedToGroup(GroupItem& group, ObjectItem& item);

		/**
		 * Used to provide this view with custom menu items
		 * @param menu The menu to append items to.
		 */
		void menuHook(QMenu& menu);

	private:
		void emitSelectionChanged();

		QVBoxLayout mLayout;	  // Layout
		ResourceModel mModel;	 // Model
		nap::qt::FilterTreeView mTreeView; // Treeview
	};
}
