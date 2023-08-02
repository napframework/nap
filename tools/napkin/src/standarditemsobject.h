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
#include <material.h>

namespace napkin
{
	//////////////////////////////////////////////////////////////////////////
	// Resource Model
	//////////////////////////////////////////////////////////////////////////

	class ComponentInstanceItem;
	class RootEntityItem;

	//////////////////////////////////////////////////////////////////////////
	// RegularResourcesItem 
	//////////////////////////////////////////////////////////////////////////

	class GroupItem;
	class ObjectItem;

	/**
	 * Used to group together all regular resources, except entities
	 */
	class RootResourcesItem : public RTTIItem
	{
		Q_OBJECT
		RTTI_ENABLE(RTTIItem)
	public:
		RootResourcesItem();
		QVariant data(int role) const override;

		/**
		 * Populate this item
		 * @param objects the objects to populate this item with
		 */
		void populate(nap::rtti::ObjectSet& objects);

		/**
		 * Clear all items
		 */
		void clear();

	Q_SIGNALS:
		/**
		 * Triggered when a new child item is added to this or a child group
		 * @param group the item the new item is added to
		 * @param item the item that is added to the group
		 */
		void childAddedToGroup(GroupItem& group, ObjectItem& item);

	private:
		/**
		 * Called when an object has been added
		 * @param obj The object that was added
		 * @param parent the parent of the object, nullptr if top level object
		 * @param selectNewObject Whether the newly created object should be selected in any views watching for object addition
		 */
		void onObjectAdded(nap::rtti::Object* obj, nap::rtti::Object* parent);

		/**
		 * Called when an object moved to another group
		 * @param object the object that moved
		 * @param oldParent old parent property
		 * @param newParent new parent property
		 */
		void onObjectReparented(nap::rtti::Object& object, PropertyPath oldParent, PropertyPath newParent);
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
		RTTI_ENABLE(RTTIItem)
	public:
		EntityResourcesItem();
		QVariant data(int role) const override;

		/**
		 * Populate this item
		 * @param objects the objects to populate this item with
		 */
		void populate(nap::rtti::ObjectSet& objects);

		/**
		 * Clear all items
		 */
		void clear();

		/**
		 * Find the (root) item for the given entity, nullptr if not found
		 * @param entity the entity to find
		 */
		const EntityItem* findEntityItem(const nap::Entity& entity) const;

	Q_SIGNALS:
		/**
		 * Signal emitted when a new item is added to an entity item
		 * @param entity the parent entity
		 * @param item the item that is added, either a EntityItem or ComponentItem
		 */
		void childAddedToEntity(EntityItem& entity, ObjectItem& item);

	private:
		/**
		 * Called when an object has been added
		 * @param obj The object that was added
		 * @param parent the parent of the object, nullptr if top level object
		 * @param selectNewObject Whether the newly created object should be selected in any views watching for object addition
		 */
		void onObjectAdded(nap::rtti::Object* obj, nap::rtti::Object* parent);
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
		RTTI_ENABLE(RTTIItem)
	public:
		/**
		 * @param object The object this item should represent
		 */
		ObjectItem(nap::rtti::Object& object);

		/**
		 * @param object The object this item should represent
		 * @param isPointer Whether this item should be displayed as a pointer/instance
		 */
		ObjectItem(nap::rtti::Object& object, bool isPointer);

		/**
		 * Get the property path this item represents
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
		nap::rtti::Object& getObject() const;

		/**
		 * @return The name of the object, which is the object ID
		 */
		virtual const QString getName() const;

		/**
		 * @return The name of the object, which is the object ID
		 */
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
		nap::rtti::Object* mObject = nullptr; // The object held by this item

	private:
		void onPropertyValueChanged(PropertyPath path);
		void onObjectRemoved(nap::rtti::Object* o);
		std::string mAbsolutePath;
		bool mIsPointer;

		/**
		 * Called just before an item is reparented
		 */
		void onObjectReparenting(nap::rtti::Object& object, PropertyPath oldParent, PropertyPath newParent);
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
		RTTI_ENABLE(ObjectItem)
	public:
		explicit EntityItem(nap::Entity& entity, bool isPointer = false);

		/**
		 * @return The entity held by this item
		 */
		nap::Entity& getEntity();

		const std::string unambiguousName() const override;

	Q_SIGNALS:
		/**
		 * Signal that is emitted when a new item is added to this entity
		 * @param entity the entity the item is added to 
		 * @param item the added item, either an EntityItem or ComponentItem.
		 */
		void childAdded(EntityItem& entity, ObjectItem& item);

	private:
		void onEntityAdded(nap::Entity* e, nap::Entity* parent);
		void onComponentAdded(nap::Component* c, nap::Entity* owner);
		void onPropertyValueChanged(const PropertyPath& path);
		void populate();
	};


	//////////////////////////////////////////////////////////////////////////
	// ComponentItem
	//////////////////////////////////////////////////////////////////////////

	class ComponentItem : public ObjectItem
	{
		Q_OBJECT
		RTTI_ENABLE(ObjectItem)
	public:
		explicit ComponentItem(nap::Component& comp);

		/**
		 * @return The component held by this item.
		 */
		nap::Component& getComponent();
	};

	//////////////////////////////////////////////////////////////////////////
	// GroupItem
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Represents a nap::IGroup.
	 * Groups together a set of objects and child -groups.
	 */
	class GroupItem : public ObjectItem
	{
		Q_OBJECT
		RTTI_ENABLE(ObjectItem)
	public:
		explicit GroupItem(nap::IGroup& group);

		/**
		 * Returns item data based on given role
		 */
		QVariant data(int role) const override;

		/**
		 * @return the resource group
		 */
		nap::IGroup& getGroup();

	Q_SIGNALS:
		/**
		 * Triggered when a new child item is added to this or a child group
		 * @param group the item the new item is added to
		 * @param item the item that is added to the group
		 */
		void childAdded(GroupItem& group, ObjectItem& item);

	private:
		/**
		 * Called when a new item is inserted into an array
		 */
		void onPropertyChildInserted(const PropertyPath& path, int index);
	};


	//////////////////////////////////////////////////////////////////////////
	// Material Item
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Represents a nap::Material that is used for shader inspection
	 */
	class MaterialItem : public ObjectItem
	{
		Q_OBJECT
		RTTI_ENABLE(ObjectItem)
	public:
		MaterialItem(nap::rtti::Object& object);
	};


	//////////////////////////////////////////////////////////////////////////
	// Shader Item
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Represents a nap::BaseShader
	 */
	class BaseShaderItem : public ObjectItem
	{
		Q_OBJECT
		RTTI_ENABLE(ObjectItem)
	public:
		BaseShaderItem(nap::rtti::Object& object);

		/**
		 * @return the shader
		 */
		const nap::BaseShader& getShader() const	{ assert(mShader != nullptr); return *mShader; }

		/**
		 * @return the shader
		 */
		nap::BaseShader& getShader()				{ assert(mShader != nullptr); return *mShader; }
			
	protected:
		void init();

	private:
		nap::BaseShader* mShader = nullptr;
	};


	//////////////////////////////////////////////////////////////////////////
	// ShaderFromFile Item
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Represents a nap::ShaderFromFile, initialized when valid
	 */
	class ShaderFromFileItem : public BaseShaderItem
	{
		Q_OBJECT
		RTTI_ENABLE(BaseShaderItem)
	public:
		/**
		 * Initializes (compiles) the shader when frag and vert path are valid
		 */
		ShaderFromFileItem(nap::rtti::Object& object);
	};


	//////////////////////////////////////////////////////////////////////////
	// ShaderFromFile Item
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Represents a nap::ShaderFromFile, initialized when valid
	 */
	class ComputeShaderFromFileItem : public BaseShaderItem
	{
		Q_OBJECT
		RTTI_ENABLE(BaseShaderItem)
	public:
		/**
		 * Initializes (compiles) the shader when frag and vert path are valid
		 */
		ComputeShaderFromFileItem(nap::rtti::Object& object);
	};


	//////////////////////////////////////////////////////////////////////////
	// Shader Item
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Represents a nap::Shader, immediately initialized
	 */
	class ShaderItem : public BaseShaderItem
	{
		Q_OBJECT
		RTTI_ENABLE(BaseShaderItem)
	public:
		/**
		 * Initializes (compiles) the shader
		 */
		ShaderItem(nap::rtti::Object& object);
	};


	//////////////////////////////////////////////////////////////////////////
	// Scene Model
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// SceneItem
	//////////////////////////////////////////////////////////////////////////

	class SceneItem : public ObjectItem
	{
		Q_OBJECT
		RTTI_ENABLE(ObjectItem)
	public:
		SceneItem(nap::Scene& scene);

		/**
		 * @return The scene held by this item
		 */
		nap::Scene& getScene();
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
		RTTI_ENABLE(ObjectItem)
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
		RTTI_ENABLE(EntityInstanceItem)
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
		RTTI_ENABLE(ObjectItem)
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
