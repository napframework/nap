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

		nap::qt::FilterTreeView& treeView();

		void select(nap::RootEntity* rootEntity, const QString& path);
	Q_SIGNALS:
		void selectionChanged(QList<PropertyPath> obj);

	private:
		void onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
		ComponentInstanceItem* resolveItem(nap::RootEntity* rootEntity, const QString& path);

		QVBoxLayout mLayout;		// Layout
		nap::qt::FilterTreeView mFilterView; // The tree view
		SceneModel mModel;		// The model for the treeview
	};
}; // napkin
