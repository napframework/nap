/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local includes
#include "actions.h"
#include "menuoptioncontroller.h"

// External includes
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
	/**^
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
		void clearItems();

		/**
		 * @return Root resources item
		 */
		RootResourcesItem& getRootResourcesItem()					{ return mObjectsItem; }

		/**
		 * @return Entity resources item
		 */
		EntityResourcesItem& getEntityResourcesItem()				{ return mEntitiesItem; }

	Q_SIGNALS:
		/**
		 * Triggered when a new child is added to an existing item
		 */
		void childAddedToGroup(GroupItem& group, ObjectItem& item);

	private:
		RootResourcesItem mObjectsItem;			// top level item that will hold objects/resources
		EntityResourcesItem  mEntitiesItem;		// top level item that will hold entities
	};

	/**
	 * A panel showing all the loaded resources in the system.
	 */
	class ResourcePanel : public QWidget
	{
		Q_OBJECT
	public:
		/**
		 * Construct widget
		 */
		ResourcePanel();

		/**
		 * Set the current selection of the treeview to the given objects
		 */
		void selectObjects(const QList<nap::rtti::Object*>& selection);

		/**
		 * @return The tree view held by this panel
		 */
		nap::qt::FilterTreeView& treeView() { return mTreeView; }

	protected:
        /**
         * Filters out and handles certain events prior to child widget
         * @param obj the object that generated the event
         * @param ev the event itself
         * @return if the filter handled the event
         */
		bool eventFilter(QObject* obj, QEvent* ev) override;

	Q_SIGNALS:
		void selectionChanged(QList<PropertyPath> obj);

	private:
		/**
		 * Clears the current model and enforces selection to be removed
		 */
		void clear();

		/**
		 * Populates the current model
		 */
		void populate();

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
		 * Called when a new item is inserted into an array
		 */
		void onChildAddedToGroup(GroupItem& group, ObjectItem& item);

		/**
		 * Called when a new item is added to an entity
		 */
		void onChildAddedToEntity(EntityItem& entity, ObjectItem& item);

		/**
		 * Called when the index of a child under a parent changes
		 */
		void onEntityIndexChanged(EntityItem& parent, ObjectItem& itemA, ObjectItem& itemB);

		/**
		 * Called when a child index changes
		 */
		void onIndexChanged(ObjectItem& parent, ObjectItem& item);

		/**
		 * Used to provide this view with custom menu items
		 * @param menu The menu to append items to.
		 */
		void menuHook(QMenu& menu);

		/**
		 * Creates possible actions for menu hook
		 */
		void createMenuCallbacks();

	private:
		void emitSelectionChanged();
		void onProjectLoaded(const nap::ProjectInfo& projectInfo);

		QVBoxLayout mLayout;							// Layout
		ResourceModel mModel;							// Model
		nap::qt::FilterTreeView mTreeView;				// Treeview
		MenuOptionController<RTTIItem> mMenuController;	// Menu option controller
	};
}
