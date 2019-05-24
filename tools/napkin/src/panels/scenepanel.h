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
		 * Reconstruct the list
		 */
		void refresh();

		/**
		 * Clears the list
		 */
		void clear();

		/**
		 * Called when an object has been added
		 * @param obj The object that was added
		 */
		void onObjectAdded(nap::rtti::Object* obj);

		/**
		 * Called when an object is about to be removed
		 * @param obj The object that will be removed
		 */
		void onObjectRemoved(nap::rtti::Object* obj);

		/**
		 * Called when an object has drastically changed
		 */
		void onObjectChanged(nap::rtti::Object* obj);

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
		 */
		void onFileClosed(const QString& filename);
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