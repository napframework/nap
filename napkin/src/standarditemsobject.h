#pragma once

#include <scene.h>

#include "actions.h"
#include "generic/filtertreeview.h"

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

		/**
		 * QStandardItem is not a QObject, so regular QObject polymorphism doesn't work.
		 * This function is supposed to solve that.
		 * See: http://doc.qt.io/qt-5/qstandarditem.html#type
		 */
		int type() const override { return StandardItemTypeID::GroupItemID; }
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
		explicit ObjectItem(nap::rtti::RTTIObject* o);

		/**
		 * QStandardItem is not a QObject, so regular QObject polymorphism doesn't work.
		 * This function is supposed to solve that.
		 * See: http://doc.qt.io/qt-5/qstandarditem.html#type
		 */
		int type() const override { return StandardItemTypeID::ObjectItemID; };

		/**
		 * Refresh
		 */
		void refresh();

		/**
		 * @return The object held by this item
		 */
		nap::rtti::RTTIObject* getObject() const;

		/**
		 * @return The name of the object.
		 */
		virtual const QString getName() const;

		void setData(const QVariant& value, int role) override;


	protected:
		nap::rtti::RTTIObject* mObject; // THe object held by this item
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
		int type() const override { return StandardItemTypeID::EntityItemID; }

		/**
		 * @return The entity held by this item
		 */
		nap::Entity* getEntity();
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
		int type() const override { return StandardItemTypeID::ComponentItemID; }

		/**
		 * @return The component held by this item.
		 */
		nap::Component& getComponent();
	};

	/**
	 * An item representing one scene
	 */
	class SceneItem : public ObjectItem
	{
	public:
		explicit SceneItem(nap::Scene& scene);

        /**
         * QStandardItem is not a QObject, so regular QObject polymorphism doesn't work.
         * This function is supposed to solve that.
         * See: http://doc.qt.io/qt-5/qstandarditem.html#type
         */
        int type() const override { return StandardItemTypeID::SceneItemID; }

	};

	/**
	 * An item that displays an entity instance
	 */
	class EntityInstanceItem : public ObjectItem
	{
	public:
		explicit EntityInstanceItem(nap::Entity& e);

        /**
         * QStandardItem is not a QObject, so regular QObject polymorphism doesn't work.
         * This function is supposed to solve that.
         * See: http://doc.qt.io/qt-5/qstandarditem.html#type
         */
        int type() const override { return StandardItemTypeID::EntityInstanceID; }

	};

} // namespace napkin
