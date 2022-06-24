/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "actions.h"
#include "rttiitem.h"

// External Includes
#include <scene.h>
#include <nap/group.h>

namespace napkin
{
	class ComponentInstanceItem;
	class RootEntityItem;

	//////////////////////////////////////////////////////////////////////////
	// RegularResourcesItem 
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Used to group together all regular resources, except entities
	 */
	class RegularResourcesItem : public RTTIItem
	{
		Q_OBJECT
	public:
		RegularResourcesItem();
		QVariant data(int role) const override;
	};


	//////////////////////////////////////////////////////////////////////////
	// EntityResourcesItem
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Used to group together all entity resources
	 */
	class EntityResourcesItem : public RTTIItem
	{
		Q_OBJECT
	public:
		EntityResourcesItem();
		QVariant data(int role) const override;
	};


	//////////////////////////////////////////////////////////////////////////
	// ObjectItem
	//////////////////////////////////////////////////////////////////////////

	/**
	 * An item representing a single nap::rtti::RTTIObject. The item will show the object's name.
	 */
	class ObjectItem : public RTTIItem
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
		 * Disambiguating index of child, index only increments when multiple children with the same name are found.
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


	//////////////////////////////////////////////////////////////////////////
	// EntityItem
	//////////////////////////////////////////////////////////////////////////

	/**
	 * An item representing an Entity
	 */
	class EntityItem : public ObjectItem
	{
		Q_OBJECT
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
	 * Represents a nap::IGroup.
	 * Groups together a set of objects and child -groups.
	 */
	class GroupItem : public ObjectItem
	{
		Q_OBJECT
	public:
		explicit GroupItem(nap::IGroup& group);

		/**
		 * Returns an invalid property path
		 */
		const PropertyPath propertyPath() const override { return PropertyPath(); }

		/**
		 * Returns item data based on given role
		 */
		QVariant data(int role) const override;

		/**
		 * @return the resource group
		 */
		nap::IGroup* getGroup();

	Q_SIGNALS:
		/**
		 * Triggered when a new child item is added to this or a child group
		 * @param group the item the new item is added to
		 * @param item the item that is added to the group
		 */
		void childAdded(GroupItem& group, ObjectItem& item);

	private:
		/**
		 * Called when an item is removed from an array
		 */
		void onPropertyChildRemoved(const PropertyPath& path, int index);

		/**
		 * Called when a new item is inserted into an array
		 */
		void onPropertyChildInserted(const PropertyPath& path, int index);
	};


	//////////////////////////////////////////////////////////////////////////
	// SceneItem
	//////////////////////////////////////////////////////////////////////////

	class SceneItem : public ObjectItem
	{
		Q_OBJECT
	public:
		explicit SceneItem(nap::Scene& scene);
	};


	//////////////////////////////////////////////////////////////////////////
	// ComponentItem
	//////////////////////////////////////////////////////////////////////////

	class ComponentItem : public ObjectItem
	{
		Q_OBJECT
	public:
		explicit ComponentItem(nap::Component& comp);

		/**
		 * @return The component held by this item.
		 */
		nap::Component& getComponent();
	};


	//////////////////////////////////////////////////////////////////////////
	// EntityInstanceItem
	//////////////////////////////////////////////////////////////////////////

	/**
	 * An item that displays an entity instance
	 */
	class EntityInstanceItem : public ObjectItem
	{
		Q_OBJECT
	public:
		explicit EntityInstanceItem(nap::Entity& e, nap::RootEntity& rootEntity);
		virtual nap::RootEntity& rootEntity() const;
		nap::Entity& entity() const;
		const PropertyPath propertyPath() const override;
		const std::string unambiguousName() const override;
	private:
		void onEntityAdded(nap::Entity* e, nap::Entity* parent);
		void onComponentAdded(nap::Component* c, nap::Entity* owner);

		nap::RootEntity& mRootEntity;
	};


	//////////////////////////////////////////////////////////////////////////
	// RootEntityItem
	//////////////////////////////////////////////////////////////////////////

	class RootEntityItem : public EntityInstanceItem
	{
	Q_OBJECT
	public:
		explicit RootEntityItem(nap::RootEntity& e);
		const PropertyPath propertyPath() const override;

		SceneItem* sceneItem();
		nap::RootEntity& rootEntity() const override;
	private:
		void onEntityAdded(nap::Entity* e, nap::Entity* parent);
		void onComponentAdded(nap::Component* c, nap::Entity* owner);

		nap::RootEntity& mRootEntity;
	};


	//////////////////////////////////////////////////////////////////////////
	// ComponentInstanceItem
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Item that displays a Component Instance,
	 * it also holds a copy of the component instance properties from the RootEntity
	 */
	class ComponentInstanceItem : public ObjectItem
	{
		Q_OBJECT
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
	};
} // namespace napkin
