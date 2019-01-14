#pragma once

#include <scene.h>

#include "actions.h"

namespace napkin
{

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
	class ObjectItem : public QStandardItem
	{
	public:
		/**
		 * @param o The object this item should represent
		 */
		explicit ObjectItem(nap::rtti::Object* o);

		/**
		 * Refresh
		 */
		void refresh();

		/**
		 * @return The object held by this item
		 */
		nap::rtti::Object* getObject() const;

		/**
		 * @return The name of the object.
		 */
		virtual const QString getName() const;

		void setData(const QVariant& value, int role) override;


	protected:
		nap::rtti::Object* mObject; // THe object held by this item
	};

	/**
	 * An item representing an Entity
	 */
	class EntityItem : public ObjectItem
	{
	public:
		explicit EntityItem(nap::Entity& entity);

		/**
		 * @return The entity held by this item
		 */
		nap::Entity* getEntity();
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

	class RootEntityItem : public ObjectItem
	{
	public:
		explicit RootEntityItem(nap::RootEntity& e);

		nap::RootEntity& rootEntity();
	private:
		nap::RootEntity* mRootEntity;
	};

	/**
	 * An item that displays an entity instance
	 */
	class EntityInstanceItem : public ObjectItem
	{
	public:
		explicit EntityInstanceItem(nap::Entity& e, RootEntityItem& rootEntityItem);

		nap::RootEntity& rootEntity();
		nap::Entity& entity() { return *dynamic_cast<nap::Entity*>(mObject); }

	private:
		RootEntityItem& mRootEntityItem;
	};

	/**
	 * Item that displays a Component Instance
	 */
	class ComponentInstanceItem : public ObjectItem
	{
	public:
		explicit ComponentInstanceItem(nap::Component& comp);
		nap::Component& component() { return *dynamic_cast<nap::Component*>(mObject); }
		nap::RootEntity& rootEntity();
	};

} // namespace napkin
