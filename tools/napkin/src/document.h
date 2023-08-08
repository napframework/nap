/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "propertypath.h"

#include <deque>
#include <nap/core.h>
#include <entity.h>
#include <nap/group.h>
#include <QString>
#include <QUndoCommand>
#include <QMap>
#include <rtti/deserializeresult.h>

namespace napkin
{
	using OwnedObjectMap = std::unordered_map<std::string, std::unique_ptr<nap::rtti::Object>>;

	/**
	 * Owns a set of objects and offers an interface to interact with those objects.
	 */
	class Document : public QObject
	{
	Q_OBJECT
	public:
		/**
		 * Construct an empty document
		 * @param core the nap core
		 */
		Document(nap::Core& core) : QObject(), mCore(core) {}

		/**
		 * Construct a new document with a specific filename and set of objects.
		 * The objects will be owned by the document.
		 * @param core nap core
		 * @param filename the name of the document
		 * @param objects unique set of objects
		 */
		Document(nap::Core& core, const QString& filename, nap::rtti::OwnedObjectList&& objects);

		~Document();

		/**
		 * @return The name of the currently opened file
		 * or an empty string if no file is open or the data hasn't been saved yet.
		 */
		const QString& getFilename() { return mCurrentFilename; }

		/**
		 * Set this document's filename
		 */
		void setFilename(const QString& filename) { mCurrentFilename = filename; }

		/**
		 * Check if changes were made to this document since the last save
		 * @return Whether this document is dirty or not
		 */
		bool isDirty() const { return !mUndoStack.isClean(); }

		/**
		 * @return A reference to all the objects (resources?) that are currently loaded.
		 */
		const OwnedObjectMap& getObjects() const { return mObjects; }

		/**
		 * Get all objects from this document, derived from the specified type.
		 * @param type The type each object has to be derived from
		 * @return All the objects in this document, derived from the provided type
		 */
		std::vector<nap::rtti::Object*> getObjects(const nap::rtti::TypeInfo& type);

		/**
		 * Get all objects from this document, derived from the specified type.
		 * @tparam T The type each object has to be derived from
		 * @return All the objects in this document, derived from the provided type
		 */
		template<typename T>
		std::vector<T*> getObjects();

		/**
		 * Retrieve an (data) object by name/id
		 * @param name The name/id of the object to find
		 * @return The found object or nullptr if none was found
		 */
		nap::rtti::Object* getObject(const std::string& name);

		/**
		 * Get an object by name and type
		 */
		nap::rtti::Object* getObject(const std::string& name, const rttr::type& type);

		/**
		 * Get an object by name and type
		 */
		template<typename T>
		T* getObject(const std::string& name) { return rtti_cast<T>(getObject(name)); }

		/**
		 * Retrieve the parent of the specified Entity
		 *
		 * TODO: Move to nap::Entity if possible
		 *
		 * @param entity The entity to find the parent from.
		 * @return The provided Entity's parent or nullptr if the Entity has no parent.
		 */
		nap::Entity* getParent(const nap::Entity& entity) const;

		/**
		 * See if an entity is a child of another.
		 * @param parentEntity The parent entity to check.
		 * @param childEntity The child entity to check for
		 * @param recursive If true, check grandchildren and so forth.
		 * @return True if the given child entity was found under the parent entity
		 */
		bool hasChild(const nap::Entity& parentEntity, const nap::Entity& childEntity, bool recursive) const;

		/**
		 * Retrieve the Entity the provided Component belongs to.
		 * TODO: Move to nap::Component if possible
		 *
		 * @param component The component of which to find the owner.
		 * @return The owner of the component
		 */
		nap::Entity* getOwner(const nap::Component& component) const;

		/**
		 * Retrieve the group the provided group belongs to,
		 * nullptr if the group is not the child of another group.
		 * @param group The group of which to find the owner
		 * @param outIndex the child index, -1 if the group isn't a child of another group
		 * @return the group, nullptr if the group isn't a child of another group
		 */
		nap::IGroup* getOwner(const nap::IGroup& group, int& outIndex) const;

		/**
		 * Retrieve the group the provided object belongs to.
		 * nullptr if the group is not the child of another group.
		 * @param group The object of which to find the owner
		 * @param outIndex the child index, -1 if the group isn't a child of another group
		 * @return the group, nullptr if the object isn't a child of another group
		 */
		nap::IGroup* getGroup(const nap::rtti::Object& object, int& outIndex) const;

		/**
		 * Set an object's name. This is similar to setting a value on it's name property,
		 * but this ensures the object has a unique name.
		 *
		 * TODO: Move to nap::rtti::Object if possible
		 */
		const std::string& setObjectName(nap::rtti::Object& object, const std::string& name, bool appenUUID = false);

		/**
		 * Add a component of the specified type to an Entity.
		 *
		 * TODO: Move to nap::Entity
		 *
		 * @param entity The entity to add the component to.
		 * @param type The type of the desired component.
		 * @return The newly created component.
		 */
		nap::Component* addComponent(nap::Entity& entity, rttr::type type);

		/**
		 * Add a component of the specified type to an Entity
		 * TODO: Move to Entity
		 * @tparam T The type of the desired component
		 * @param entity The entity to add the component to.
		 * @return The newly created component
		 */
		template<typename T>
		T* addComponent(nap::Entity& entity) { return rtti_cast<T>(addComponent(entity, RTTI_OF(T))); }

		/**
		 * Get an Entity's first component of the given type.
		 * TODO: Move to Entity
		 * @param entity The entity that owns the component
		 * @param componentType The type of component to look for
		 * @return The component if found, nullptr otherwise
		 */
		nap::Component* getComponent(nap::Entity& entity, rttr::type componentType);

		/**
		 * Remove an Component from an Entity
		 * @param component The component to remove from the entity.
		 */
		void removeComponent(nap::Component& component);

		/**
		 * Add an object of the specified type.
		 * @param type The type of the desired object.
		 * @param parent The parent of the object: In the case of Entity, this will be its new parent.
		 * In the case of Component, this is going to be the owning Entity.
		 * @return The newly created object
		 */
		nap::rtti::Object* addObject(rttr::type type, nap::rtti::Object* parent = nullptr, const std::string& name = std::string());

		/**
		 * Moves an object to a new group.
		 * Removes the object from the current group if necessary.
		 * @param object the object to move, group or object
		 * @param currentPath the property path it is currently parented under, invalid path if item is not parented
		 * @param newParent the new property path to move the object to, invalid path item if it is not to be parented
		 */
		void reparentObject(nap::rtti::Object& object, const PropertyPath& currentPath, const PropertyPath& newPath);

		/**
		 * Add an object of the specified type
		 * @tparam T
		 * @param parent
		 * @return
		 */
		template<typename T>
		T* addObject(nap::rtti::Object* parent = nullptr, const std::string& name = std::string())
		{
			return reinterpret_cast<T*>(addObject(RTTI_OF(T), parent, name));
		}

		/**
		 * Add and entity to the document
		 * @param parent The parent of the newly created entity or nullptr
		 * @return The newly created Entity
		 */
		nap::Entity& addEntity(nap::Entity* parent = nullptr, const std::string& name = "");

		/**
		 * Obliterate the specified object and its dependents
		 * @param object The object to be deleted.
		 */
		void removeObject(nap::rtti::Object& object);

		/**
		 * If the object with the specified name was found, nuke it from orbit.
		 */
		void removeObject(const std::string& name);

		/**
		 * Remove all overrides for the specified object
		 */
		void removeInstanceProperties(nap::rtti::Object& object);

		/**
		 * Remove all overrides for the specified object, but only in the specified scene
		 */
		void removeInstanceProperties(nap::Scene& scene, nap::rtti::Object& object);

		/**
		 * Remove instance properties for a parentEntity / childEntity combo
		 * @param path property path, expected to be a child of another entity
		 */
		void removeInstanceProperties(PropertyPath path);

		/**
		 * Get all components recursively starting from the given object
		 * If the given object is not a component or entity the list is empty
		 */
		QList<nap::Component*> getComponentsRecursive(nap::rtti::Object& object);

		/**
		 * Recursively iterate an Entity's children
		 */
		void recurseChildren(nap::Entity& entity, std::function<void(nap::Entity& child)>);

		/**
		 * Remove all entity instances from a scene, note that a Scene may contain the same entity multiple times.
		 * @param scene The Scene to remove the entity from
		 * @param entity The entity to remove from the scene
		 */
		void removeEntityFromScene(nap::Scene& scene, nap::Entity& entity);

		/**
		 * Remove an Entity from a scene at the specified index
		 * @param scene The Scene to remove the Entity from
		 * @param index The index of the Entity to be removed
		 */
		void removeEntityFromScene(nap::Scene& scene, size_t index);

		/**
		 * Add an entity to a scene (at root level)
		 * @param scene The Scene to add the Entity to
		 * @param entity The Entity to add to the Scene
		 * @return the index at which the entity was added
		 */
		size_t addEntityToScene(nap::Scene& scene, nap::Entity& entity);

		/**
		 * Add an entity to another Entity's children list, you can have multiple of the same children
		 * @param parent The Entity to add the child Entity to
		 * @param child The Entity to add to the other
		 * @return the resultin index of the Entity
		 */
		size_t addChildEntity(nap::Entity& parent, nap::Entity& child);

		/**
		 * Remove a child entity from another Entity's children
		 *
		 * 		WARNING: This will NOT take care of removing and patching up instance properties
		 *
		 * @param parent The parent Entity to remove the child from
		 * @param childIndex The index of the child Entity to be removed
		 */
		void removeChildEntity(nap::Entity& parent, size_t childIndex);

		/**
		 * Remove an object or property
		 * @param path The path that determines what to remove
		 */
		void remove(const PropertyPath& path);

		/**
		 * Return a RootEntities in a scene that represent the specified entity.
		 * For more explanation see RootEntity
		 */
		QList<nap::RootEntity*> getRootEntities(nap::Scene& scene, nap::rtti::Object& object);

		/**
		 * Retrieve all properties referring to the given object.
		 * @param targetObject The object that is being referred to.
		 * @return A list of properties pointing to the given object.
		 */
		QList<PropertyPath> getPointersTo(const nap::rtti::Object& targetObject, bool excludeArrays, bool excludeParent, bool excludeInstanceProperties = true);

		/**
		 * Add an element to the end of an array
		 * The propertyValueChanged signal will be emitted.
		 * @param path The path to the array property to add the element to
		 * @return The index of the newly created element
		 */
		int arrayAddValue(const PropertyPath& path);

		/**
		 * Add an existing pointer to the array
		 * The propertyValueChanged signal will be emitted.
		 * @param path The path to the array
		 * @param object The object pointer to addd
		 * @param index The index at which to add the new element
		 * @return The index at which the element lives.
		 */
		size_t arrayAddExistingObject(const PropertyPath& path, nap::rtti::Object* object, size_t index);

		/**
		 * Create an object of the specified type and add it to the array
		 * The propertyValueChanged signal will be emitted.
		 * @param path The path to the array
		 * @param type The type of object to create
		 * @param index The index at which to add the new element
		 * @return The index of the inserted object
		 */
		int arrayAddNewObject(const PropertyPath& path, const nap::rtti::TypeInfo& type, size_t index);

		/**
		 * Remove an element from an array
		 * Emits propertyValueChanged & propertyChildRemoved signals
		 * @param path The path pointing to the array
		 * @param index The index of the element to remove
		 */
		void arrayRemoveElement(const PropertyPath& path, size_t index);

		/**
		 * Remove an element from a group
		 * Emits propertyValueChanged & propertyChildRemoved signals
		 * @param group The group to remove the element from
		 * @param arrayProperty the array property that contains the element
		 * @param index The index of the element to remove
		 */
		void groupRemoveElement(nap::IGroup& group, rttr::property arrayProperty, size_t index);

		/**
		 * Move an item within an array. If \p fromIndex is greater than \p toIndex,
		 * \p toIndex is considered to be the destination index <b>before</b> the move happens.
		 * The propertyValueChanged signal will be emitted.
		 * @param path The path to the array property
		 * @param fromIndex The index of the element to move
		 * @param toIndex The destination index of the element
		 * @return The resulting index of the element
		 */
		size_t arrayMoveElement(const PropertyPath& path, size_t fromIndex, size_t toIndex);

		/**
		 * Get an element from an array
		 * @param path The path to the array property
		 * @param index The index of the element to retrieve
		 * @return The resulting value
		 */
		nap::rtti::Variant arrayGetElement(const PropertyPath& path, size_t index) const;

		/**
		 * Get an element from an array
		 * @tparam T cast to the specified type.
		 * @param path The path to the array property
		 * @param index The index of the element to retrieve
		 * @return The resulting value
		 */
		template<typename T>
		T arrayGetElement(const PropertyPath& path, size_t index) { return arrayGetElement(path, index).convert<T>(); }

		/**
		 * See if this object is being pointed to by an embedded pointer.
		 * @param obj The object potentially being pointed to
		 * @return true if this object is being pointed to by an embedded pointer
		 */
		bool isPointedToByEmbeddedPointer(const nap::rtti::Object& obj);

		/**
		 * If this object is pointed to by an embedded pointer, return the object that declares that pointer.
		 * Return nullptr when there is no such object.
		 * @param obj The object being pointed to.
		 * @return The given (embedded) object's owner.
		 */
		nap::rtti::Object* getEmbeddedObjectOwner(const nap::rtti::Object& obj);

		/**
		 * If this object retrieve the [property] path pointing to this object.
		 * @param obj The object being pointed to
		 * @return The property path pointing to the given object, or an invalid path
		 */
		PropertyPath getEmbeddedObjectOwnerPath(const nap::rtti::Object& obj);

		/**
		 * Retrieve objects embedded in the given object.
		 * @param owner The object that declares embedded pointers
		 * @return A list of objects, owned by the given object.
		 */
		std::vector<nap::rtti::Object*> getEmbeddedObjects(nap::rtti::Object& owner);

		/**
		 * Get the absolute path of an object
		 * @param obj The object to get the path to
		 */
		std::string absoluteObjectPath(const nap::rtti::Object& obj) const;

		/**
		 * Retrieve an absolute object path as a list
		 * @param obj The object to get the path to
		 * @param result A list to store the result into
		 */
		void absoluteObjectPathList(const nap::rtti::Object& obj, std::deque<std::string>& result) const;

		/**
		 * Get a relative path from an object to another object as a string
		 * @param from The starting object
		 * @param to The object to point to
		 * @param result The relative path :)
		 */
		std::string relativeObjectPath(const nap::rtti::Object& origin, const nap::rtti::Object& target) const;

		/**
		 * Get a relative path from an object to another object as a list
		 * @param from The starting object
		 * @param to The object to point to
		 * @param result The relative path
		 */
		void relativeObjectPathList(const nap::rtti::Object& origin, const nap::rtti::Object& target,
									std::deque<std::string>& result) const;

		/**
		 * Execute the specified command and push the provided command onto the undostack.
		 * @param cmd The command to be executed
		 */
		void executeCommand(QUndoCommand* cmd);

		/**
		 * Forward to undostack
		 */
		void undo() { getUndoStack().undo(); }

		/**
		 * Forward to undostack
		 */
		void redo() { getUndoStack().redo(); }

		/**
		 * @return This document's undo stack
		 */
		QUndoStack& getUndoStack() { return mUndoStack; }

	Q_SIGNALS:
		/**
		 * Qt Signal
		 * Invoked when an Entity has been added to the system
		 * @param newEntity The newly added Entity
		 * @param parent The parent the new Entity was added to
		 */
		void childEntityAdded(nap::Entity* newEntity, nap::Entity* parent = nullptr);

		/**
		 * Qt Signal
		 * Invoked when a Component has been added to the system
		 * @param comp
		 * @param owner
		 */
		void componentAdded(nap::Component* comp, nap::Entity* owner);

		/**
		 * Qt Signal
		 * Invoked after any object has been added (this includes Entities and Groups)
		 * @param obj The newly added object
		 * @param parent The parent item of the newly added object, can be nullptr
		 */
		void objectAdded(nap::rtti::Object* obj, nap::rtti::Object* parent);

		/**
		 * Qt Signal
		 * Invoked after an object has changed drastically
		 */
		void objectChanged(nap::rtti::Object* obj);

		/**
		 * Qt Signal
		 * Invoked just before an object is removed. This includes entities, components and regular resources.
		 * The item, including all of it's embedded children, are still part of the document. 
		 * @param object The object about to be removed
		 */
		void removingObject(nap::rtti::Object* object);

		/**
		 * Qt Signal
		 * Invoked just after a resource is removed, but before it is destroyed.
		 * This including entities, components and regular resources.
		 * The object has been removed from the document, but not yet destroyed!
		 * The embedded child objects, including components and child groups,
		 * have been destroyed and removed from the document.
		 * @param object The object about to be removed
		 */
		void objectRemoved(nap::rtti::Object* object);

		/**
		 * Qt Signal
		 * Invoked after an object has moved to a new group
		 * @param object The object that moved to a new group
		 * @param oldParent The previous parent (array) property, invalid if it had no parent
		 * @param newParent The new parent (array) property, invalid if not attached to a new parent
		 */
		void objectReparented(nap::rtti::Object& object, PropertyPath oldParent, PropertyPath newParent);

		/**
		 * Qt Signal
		 * Invoked before an object is moved to a new group
		 * @param object The object that is moved to a new group
		 * @param oldParent The current parent (array) property, invalid if it had no parent
		 * @param newParent The new parent (array) property, invalid if not attached to a new parent
		 */
		void objectReparenting(nap::rtti::Object& object, PropertyPath oldParent, PropertyPath newParent);

		/**
		 * Qt Signal
		 * Invoked after an object has been renamed.
		 * @param object the object that has been renamed
		 * @param oldName old (now invalid) object property name
		 * @param newName new (now valid) object property name
		 */
		void objectRenamed(nap::rtti::Object& object, const std::string& oldName, const std::string& newName);

		/**
		 * Qt Signal
		 * Invoked just after a property's value has changed
		 * @param path The path to the property that has changed
		 */
		void propertyValueChanged(const PropertyPath& path);

		/**
		 * Qt Signal
		 * Invoked just after a property child has been inserted
		 * @param path The path to the parent of the newly added child
		 * @param childIndex The index of the newly added child
		 */
		void propertyChildInserted(const PropertyPath& parentPath, size_t childIndex);

		/**
		 * Qt Signal
		 * Invoked just after a property child has been removed
		 * @param path The path to the parent of the newly added child
		 * @param childIndex The index of the child that was removed
		 */
		void propertyChildRemoved(const PropertyPath& parentPath, size_t childIndex);

	private:
		nap::Core& mCore;							// nap's core
		OwnedObjectMap mObjects;					// The objects in this document
		QString mCurrentFilename;					// This document's filename
		QUndoStack mUndoStack;						// This document's undostack

		/**
		 * @return unique object name
		 */
		std::string getUniqueName(const std::string& suggestedName, const nap::rtti::Object& object, bool useUUID);
	};


	//////////////////////////////////////////////////////////////////////////
	// Template Definitions
	//////////////////////////////////////////////////////////////////////////

	template<typename T>
	std::vector<T*> Document::getObjects()
	{
		auto objects = getObjects(RTTI_OF(T));
		std::vector<T*> ret; 
		ret.reserve(objects.size());
		for (auto& obj : objects)
		{
			ret.emplace_back(static_cast<T*>(obj));
		}
		return ret;
	}
}
