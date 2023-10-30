/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <QtWidgets/QLayout>
#include <QtWidgets/QWidget>
#include <napqt/filtertreeview.h>
#include <scene.h>
#include <propertypath.h>
#include <standarditemsobject.h>


namespace napkin
{
	class RootEntityItem;

    /**
	 * Provides the view with scene data
	 */
	class SceneModel : public QStandardItemModel
	{
		Q_OBJECT
	public:
		SceneModel();

		/**
		 * Find root entity item based on root entity
		 */
		RootEntityItem* rootEntityItem(nap::RootEntity& rootEntity) const;

		/**
		 * @return all the top level scene items
		 */
		const std::vector<SceneItem*>& getScenes() const							{ return mSceneItems; }

	Q_SIGNALS:
		/**
		 * Signal emitted when the model is re-populated based on a change
		 * @param scenes the new scenes
		 */
		void populated(const std::vector<SceneItem*>& scenes);

	private:
		/**
		 * Clears the list
		 */
		void clear();

		/**
		 * Populates the list
		 */
		void populate();

		/**
		 * Called when an object has been added
		 * @param obj The object that was added
		 */
		void onObjectAdded(nap::rtti::Object* obj);

		/**
		 * Called when an object has drastically changed
		 */
		void onObjectChanged(nap::rtti::Object* obj);

		/**
		 * Called when an object has been  removed
		 */
		void onObjectRemoved(nap::rtti::Object* obj);

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
		 * @param filename The name of the document
		 */
		void onFileClosing(const QString& filename);

		std::vector<SceneItem*> mSceneItems;
	};

	/**
	 * This panel displays the currently loaded scene
	 */
	class ScenePanel : public QWidget
	{
		Q_OBJECT
	public:
		ScenePanel();

		/**
		 * Let this view add to the context menu
		 */
		void menuHook(QMenu& menu);

		/**
		 * @return the scene tree view
		 */
		nap::qt::FilterTreeView& treeView();

		/**
		 * Select an entity based on the given pointer and object path
		 * @param rootEntity the top level entity
		 * @param path child entity path
		 */
		void select(nap::RootEntity* rootEntity, const QString& path);

	protected:
		bool eventFilter(QObject* obj, QEvent* ev) override;

	Q_SIGNALS:
		void selectionChanged(QList<PropertyPath> obj);

	private:
		void onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
		void onModelPopulated(const std::vector<SceneItem*>& scenes);
		ComponentInstanceItem* resolveItem(nap::RootEntity* rootEntity, const QString& path);

		QVBoxLayout mLayout;					// Layout
		nap::qt::FilterTreeView mFilterView;	// The tree view
		SceneModel mModel;						// The model for the tree view
	};
}; // napkin
