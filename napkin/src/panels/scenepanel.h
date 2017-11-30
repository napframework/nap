#pragma once

#include <QtWidgets/QLayout>
#include <QtWidgets/QWidget>
#include <generic/filtertreeview.h>
#include <scene.h>


namespace napkin
{


    /**
	 * Provides the view with scene data
	 */
	class SceneModel : public QStandardItemModel
	{
		Q_OBJECT
	public:
		SceneModel();

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
		void onComponentAdded(nap::Component& comp, nap::Entity& owner);

		/**
		 * Called when an object has been added
		 * @param obj The object that was added
		 */
		void onObjectAdded(nap::rtti::RTTIObject& obj);

		/**
		 * Called when an object is about to be removed
		 * @param obj The object that will be removed
		 */
		void onObjectRemoved(nap::rtti::RTTIObject& obj);

		/**
		 * Called when a new file was created.
		 */
		void onNewFile();

		/**
		 * Called just after a file has been opened
		 * @param filename The name of the file that was opened
		 */
		void onFileOpened(const QString& filename);
	};

	/**
	 * This panel displays the currently loaded scene
	 */
	class ScenePanel : public QWidget
	{
		Q_OBJECT
	public:
		ScenePanel();

	private:
		QVBoxLayout mLayout;		// Layout
		FilterTreeView mFilterView; // The tree view
		SceneModel mModel;		// The model for the treeview
	};
}; // napkin