#pragma once

#include <scene.h>

#include "actions.h"

namespace napkin
{
	class ComponentInstanceItem;
	class RootEntityItem;

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
	};

	/**
	 * An item representing a single nap::rtti::RTTIObject. The item will show the object's name.
	 */
	class ObjectItem : public QObject, public QStandardItem
	{
		Q_OBJECT
	public:
		/**
		 * @param o The object this item should represent
		 * @param isPointer Whether this item should be displayed as a pointer/instance
		 */
		explicit ObjectItem(nap::rtti::Object* o, bool isPointer = false);

		/**
		 * @return true if this item is representing a pointer instead of the actual object
		 */
		bool isPointer() const;

		/**
		 * Refresh
		 */
		void refresh();

		/**
		 * @return The parent QStandardItem if one exists
		 */
		QStandardItem* parentItem() { return QStandardItem::parent(); }

		/**
		 * @return The object held by this item
		 */
		nap::rtti::Object* getObject() const;

		/**
		 * @return The name of the object.
		 */
		virtual const QString getName() const;

		/**
		 * Override from QStandardItem
		 */
		void setData(const QVariant& value, int role) override;

		/**
		 * Override from QStandardItem
		 */
		QVariant data(int role) const override;

		/**
		 * Remove all children of this item
		 */
		void removeChildren();

	protected:
		nap::rtti::Object* mObject; // THe object held by this item

	private:
		void onPropertyValueChanged(PropertyPath path);
		void onObjectRemoved(nap::rtti::Object* o);

		bool mIsPointer;
	};

	/**
	 * An item representing an Entity
	 */
	class EntityItem : public ObjectItem
	{
	public:
		explicit EntityItem(nap::Entity& entity, bool isPointer = false);

		/**
		 * @return The entity held by this item
		 */
		nap::Entity* getEntity();

		/**
		 * Index of the given item's Entity under this item's Entity
		 */
		int childEntityIndex(EntityItem& childEntityItem);

	private:
		void onEntityAdded(nap::Entity* e, nap::Entity* parent);
		void onComponentAdded(nap::Component* c, nap::Entity* owner);
		void onPropertyValueChanged(const PropertyPath& path);

	};

	/**
	 * An item representing one scene
	 */
	class SceneItem : public ObjectItem
	{
	public:
		explicit SceneItem(nap::Scene& scene);

	};

	/**
	 * And item representing a Component
	 */
	class ComponentItem : public ObjectItem
	{
	public:
		explicit ComponentItem(nap::Component& comp);

		/**
		 * @return The component held by this item.
		 */
		nap::Component& getComponent();
	};

	/**
	 * An item that displays an entity instance
	 */
	class EntityInstanceItem : public ObjectItem
	{
		Q_OBJECT
	public:
		explicit EntityInstanceItem(nap::Entity& e, RootEntityItem& rootEntityItem);
		QString instanceName();
		nap::RootEntity& rootEntity();
		nap::Entity& entity() const { return *dynamic_cast<nap::Entity*>(mObject); }
		EntityInstanceItem* parentEntityInstanceItem() { return dynamic_cast<EntityInstanceItem*>(parentItem()); }
		int componentIndex(const ComponentInstanceItem& item);
		int childIndex(const EntityInstanceItem& item);
		const PropertyPath path() const;

	private:
		void onObjectRemoved(nap::rtti::Object* o);
		void onEntityAdded(nap::Entity* e, nap::Entity* parent);
		void onComponentAdded(nap::Component* c, nap::Entity* owner);

		RootEntityItem& mRootEntityItem;
	};

	class RootEntityItem : public EntityInstanceItem
	{
	Q_OBJECT
	public:
		explicit RootEntityItem(nap::RootEntity& e);

		SceneItem* sceneItem() { return dynamic_cast<SceneItem*>(QStandardItem::parent()); }
		nap::RootEntity& rootEntity();

	private:
		void onEntityAdded(nap::Entity* e, nap::Entity* parent);
		void onComponentAdded(nap::Component* c, nap::Entity* owner);


		nap::RootEntity* mRootEntity;
	};


	/**
	 * Item that displays a Component Instance
	 */
	class ComponentInstanceItem : public ObjectItem
	{
	public:
		explicit ComponentInstanceItem(nap::Component& comp, RootEntityItem& rootEntityItem);
		nap::Component& component() { return *dynamic_cast<nap::Component*>(mObject); }
		nap::RootEntity& rootEntity();
		std::string componentPath();
	private:
		RootEntityItem& mEntityItem;
	};

} // namespace napkin
