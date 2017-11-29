#pragma once

#include "actions.h"
#include "appcontext.h"
#include "generic/filtertreeview.h"
#include "generic/napgeneric.h"
#include <QStandardItemModel>
#include <nap/resourcemanager.h>


namespace nap
{
	namespace rtti
	{
		class RTTIObject;
	}
}
namespace napkin
{
	enum ResourcePanelPanelStandardItemTypeID
	{
		GroupItemTypeID = 1,
		ObjectItemTypeID = 2,
		EntityItemTypeID = 3,
		ComponentItemTypeID = 4
	};


	/**
	 * An empty item for grouping purposes.
	 */
	class GroupItem : public QStandardItem
	{
	public:
		/**
		 * @param name The name of the group
		 */
		explicit GroupItem(const QString& name);

		/**
		 * QStandardItem is not a QObject, so regular QObject polymorphism doesn't work.
		 * This function is supposed to solve that.
		 * See: http://doc.qt.io/qt-5/qstandarditem.html#type
		 */
		int type() const override;
	};

	/**
	 * An item representing a single nap::rtti::RTTIObject. The item will show the object's name.
	 */
	class ObjectItem : public QStandardItem
	{
	public:
		/**
		 * @param o The object this item should represent
		 */
		explicit ObjectItem(nap::rtti::RTTIObject& o);

		/**
		 * QStandardItem is not a QObject, so regular QObject polymorphism doesn't work.
		 * This function is supposed to solve that.
		 * See: http://doc.qt.io/qt-5/qstandarditem.html#type
		 */
		int type() const override;

		/**
		 * Refresh
		 */
		void refresh();

		/**
		 * @return The object held by this item
		 */
		nap::rtti::RTTIObject& object() const;

		/**
		 * @return The name of the object.
		 */
		virtual const QString name() const;

	protected:
		nap::rtti::RTTIObject& mObject; // THe object held by this item
	};

	/**
	 * An item representing an Entity
	 */
	class EntityItem : public ObjectItem
	{
	public:
		explicit EntityItem(nap::Entity& entity);

		/**
		 * QStandardItem is not a QObject, so regular QObject polymorphism doesn't work.
		 * This function is supposed to solve that.
		 * See: http://doc.qt.io/qt-5/qstandarditem.html#type
		 */
		int type() const override;

		/**
		 * @return The entity held by this item
		 */
		nap::Entity& entity();
	};

	/**
	 * And item representing a Component
	 */
	class ComponentItem : public ObjectItem
	{
	public:
		explicit ComponentItem(nap::Component& comp);

		/**
		 * QStandardItem is not a QObject, so regular QObject polymorphism doesn't work.
		 * This function is supposed to solve that.
		 * See: http://doc.qt.io/qt-5/qstandarditem.html#type
		 */
		int type() const override;

		/**
		 * @return The component held by this item.
		 */
		nap::Component& component();
	};

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
	};

	/**
	 * A panel showing all the loaded resources in the system.
	 */
	class ResourcePanel : public QWidget
	{
		Q_OBJECT
	public:
		ResourcePanel();

	Q_SIGNALS:

		void selectionChanged(QList<nap::rtti::RTTIObject*>& obj);

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

		/**
		 * Called when the global selection was changed
		 * @param selected The items that were selected
		 * @param deselected The items that were deselected
		 */
		void onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

		/**
		 * Finds a model item that keeps track of the specified object
		 * @param obj The object to look for in the model items
		 * @return The model item or nullptr if no such item was found
		 */
		class ObjectItem* findItem(const nap::rtti::RTTIObject& obj);

		/**
		 * @return The instances currently selected in the view.
		 */
		std::vector<rttr::instance> selectedInstances() const;

		/**
		 * Used to provide this view with custom menu items
		 * @param menu The menu to append items to.
		 */
		void menuHook(QMenu& menu);

	private:
		QVBoxLayout mLayout;	  // Layout
		ResourceModel mModel;	 // Model
		FilterTreeView mTreeView; // Treeview
	};
}