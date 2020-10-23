/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "actions.h"
#include <scene.h>

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
		enum class GroupType {
			Resources, Entities
		};

		/**
		 * @param name The name of the group
		 */
		explicit GroupItem(const QString& name, GroupItem::GroupType t);

		GroupType groupType() const { return mType; }
	private:
		GroupType mType;
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
		 * Get the propertypath this item represents
		 */
		virtual const PropertyPath propertyPath() const;

		std::string absolutePath() const;

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
		QStandardItem* parentItem() const { return QStandardItem::parent(); }

		/**
		 * @return The object held by this item
		 */
		nap::rtti::Object* getObject() const;

		/**
		 * @return The name of the object.
		 */
		virtual const QString getName() const;

		virtual const std::string unambiguousName() const;

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

		/**
		 * The disambiguating name for this [component].
		 * eg.
		 * 		MyComponent:4
		 *
		 * TODO: this should not reside in the GUI code
		 */
		QString instanceName() const;

		/**
		 * The instance path from the scene's RootEntity to the component
		 * eg.
		 * 		./MyEntityA:2/MyEntityC:0/MyComponent:3
		 *
		 * TODO: this should not reside in the GUI code
		 */
		std::string componentPath() const;

		/**
		 * Index of the given child item under this item
		 */
		int childIndex(const ObjectItem& childItem) const;

		/**
		 * Disabmiguating index of child, index only increments when multiple children with the same name are found.
		 */
		int nameIndex(const ObjectItem& childItem) const;

	protected:

		nap::rtti::Object* mObject; // THe object held by this item

	private:
		void onPropertyValueChanged(PropertyPath path);
		void onObjectRemoved(nap::rtti::Object* o);
		std::string mAbsolutePath;
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

		const std::string unambiguousName() const override;

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
		explicit EntityInstanceItem(nap::Entity& e, nap::RootEntity& rootEntity);
		virtual nap::RootEntity& rootEntity() const;
		nap::Entity& entity() const { return *dynamic_cast<nap::Entity*>(mObject); }
		const PropertyPath propertyPath() const override;
		const std::string unambiguousName() const override;
	private:
		void onEntityAdded(nap::Entity* e, nap::Entity* parent);
		void onComponentAdded(nap::Component* c, nap::Entity* owner);

		nap::RootEntity& mRootEntity;
	};

	class RootEntityItem : public EntityInstanceItem
	{
	Q_OBJECT
	public:
		explicit RootEntityItem(nap::RootEntity& e);
		const PropertyPath propertyPath() const override;

		SceneItem* sceneItem() { return dynamic_cast<SceneItem*>(QStandardItem::parent()); }
		nap::RootEntity& rootEntity() const override;
	private:
		void onEntityAdded(nap::Entity* e, nap::Entity* parent);
		void onComponentAdded(nap::Component* c, nap::Entity* owner);

		nap::RootEntity& mRootEntity;
	};


	/**
	 * Item that displays a Component Instance,
	 * it also holds a copy of the componentinstanceproperties from the RootEntity
	 */
	class ComponentInstanceItem : public ObjectItem
	{
	public:
		explicit ComponentInstanceItem(nap::Component& comp, nap::RootEntity& rootEntity);
		const PropertyPath propertyPath() const override;
		nap::Component& component() const;
		nap::RootEntity& rootEntity() const;
		QVariant data(int role) const override;
	private:
		nap::ComponentInstanceProperties* instanceProperties() const;
		bool hasInstanceProperties() const;

		nap::RootEntity& mRootEntity;

		mutable bool mInstancePropertiesResolved = false;

		// This is a copy of the instanceproperties on the root entity
		mutable nap::ComponentInstanceProperties mInstanceProperties;

		QColor mOverrideColor;
	};

} // namespace napkin
