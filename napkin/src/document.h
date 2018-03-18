#pragma once

#include <rtti/deserializeresult.h>
#include <QtCore/QString>
#include <entity.h>
#include <QtWidgets/QUndoCommand>
#include <nap/core.h>
#include <generic/propertypath.h>

namespace napkin
{
	/**
	 * A document 'owns' a bunch of objects, it's own undostack
	 */
	class Document : public QObject
	{
		Q_OBJECT
	public:
		Document(nap::Core& core) : QObject(), mCore(core)  {}

		Document(nap::Core& core, const QString& filename, nap::rtti::OwnedObjectList objects)
				: QObject(), mCore(core), mCurrentFilename(filename), mObjects(std::move(objects)) {}
		
		~Document();
		
		/**
		 * @return The name of the currently opened file
		 * or an empty string if no file is open or the data hasn't been saved yet.
		 */
		const QString& getCurrentFilename()	{ return mCurrentFilename; }

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
		 * @return All the objects (resources?) that are currently loaded.
		 */
		nap::rtti::OwnedObjectList& getObjects() { return mObjects; }

		/**
		 * @return All the objects (resources?) that are currently loaded.
		 */
		const nap::rtti::OwnedObjectList& getObjects() const { return mObjects; }

		/**
		 * @return All the objects (resources?) that are currently loaded.
		 */
		nap::rtti::ObjectList getObjectPointers();

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
		nap::Entity* getParent(const nap::Entity& entity);

		/**
		 * Retrieve the Entity the provided Component belongs to.
		 *
		 * TODO: Move to nap::Component if possible
		 *
		 * @param component The component of which to find the owner.
		 * @return The owner of the component
		 */
		nap::Entity* getOwner(const nap::Component& component);

		/**
		 * Set an object's name. This is similar to setting a value on it's name property,
		 * but this ensures the object has a unique name.
		 *
		 * TODO: Move to nap::rtti::Object if possible
		 */
		const std::string& setObjectName(nap::rtti::Object& object, const std::string& name);

		/**
		 * Add a component of the specified type to an Entity.
		 *
		 * TODO: Move to nap::Entity if possible
		 *
		 * @param entity The entity to add the component to.
		 * @param type The type of the desired component.
		 * @return The newly created component.
		 */
		nap::Component* addComponent(nap::Entity& entity, rttr::type type);

		/**
		 * Add a component of the specified type to an Entity
		 * @tparam T The type of the desired component
		 * @param entity The entity to add the component to.
		 * @return The newly created component
		 */
		template<typename T>
		T* addComponent(nap::Entity& entity) { return rtti_cast<T>(addComponent(entity, RTTI_OF(T))); }

		/**
		 * Add an object of the specified type.
		 * @param type The type of the desired object.
		 * @param parent The parent of the object: In the case of Entity, this will be its new parent.
		 * 	In the case of Component, this is going to be the owning Entity.
		 * @return The newly created object
		 */
		nap::rtti::Object* addObject(rttr::type type, nap::rtti::Object* parent = nullptr);

		/**
		 * Add an object of the specified type
		 * @tparam T
		 * @param parent
		 * @return
		 */
		template<typename T>
		T* addObject(nap::rtti::Object* parent = nullptr) { return rtti_cast<T>(addObject(RTTI_OF(T), parent)); }

		/**
		 * Add and entity to the document
		 * @return The newly created Entity
		 */
		nap::Entity& addEntity();

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
		 * Remove an entity from a scene, note that a Scene may contain the same entity multiple times.
		 * @param scene The Scene to remove the entity from
		 * @param entity The entity to remove from the scene
		 */
		void removeEntityFromScene(nap::Scene& scene, nap::Entity& entity);

		/**
		 * Add an entity to a scene (at root level)
		 * @param scene The Scene to add the Entity to
		 * @param entity The Entity to add to the Scene
		 * @return the index at which the entity was added
		 */
		size_t addEntityToScene(nap::Scene& scene, nap::Entity& entity);

		/**
		 * Retrieve all properties referring to the given object.
		 * @param obj The object that is being referred to.
		 * @return A list of properties pointing to the given object.
		 */
		QList<PropertyPath> getPointersTo(const nap::rtti::Object& obj, bool excludeArrays);

		/**
		 * Add an element to an array
		 * The propertyValueChanged signal will be emitted.
		 * @param path The path to the array property to add the element to
		 * @param index The index at which to add the new element, provide -1 to add to the end
		 * @return The index of the newly created element
		 */
		size_t arrayAddValue(const PropertyPath& path, size_t index);

		/**
		 * Add an element to the end of an array
		 * The propertyValueChanged signal will be emitted.
		 * @param path The path to the array property to add the element to
		 * @return The index of the newly created element
		 */
		size_t arrayAddValue(const PropertyPath& path);

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
		 * Add an existing pointer to the end of an array
		 * The propertyValueChanged signal will be emitted.
		 * @param path The path to the array
		 * @param object The object pointer to addd
		 * @return The index at which the element lives.
		 */
		size_t arrayAddExistingObject(const PropertyPath& path, nap::rtti::Object* object);

		/**
		 * Create an object of the specified type and add it to the array
		 * The propertyValueChanged signal will be emitted.
		 * @param path The path to the array
		 * @param type The type of object to create
		 * @param index The index at which to add the new element
		 * @return The index of the inserted object
		 */
		size_t arrayAddNewObject(const PropertyPath& path, const nap::rtti::TypeInfo& type, size_t index);

		/**
		 * Create an object of the specified type and add it to the end of the array
		 * The propertyValueChanged signal will be emitted.
		 * @param path The path to the array
		 * @param type The type of object to create
		 * @return The index of the inserted object
		 */
		size_t arrayAddNewObject(const PropertyPath& path, const nap::rtti::TypeInfo& type);

		/**
		 * Remove an element from an array
		 * The propertyValueChanged signal will be emitted.
		 * @param path The path pointing to the array
		 * @param index The index of the element to remove
		 */
		void arrayRemoveElement(const PropertyPath& path, size_t index);

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
		void entityAdded(nap::Entity* newEntity, nap::Entity* parent = nullptr);

		/**
		 * Qt Signal
		 * Invoked when a Component has been added to the system
		 * @param comp
		 * @param owner
		 */
		void componentAdded(nap::Component& comp, nap::Entity& owner);

		/**
		 * Qt Signal
		 * Invoked after any object has been added (this includes Entities)
		 * @param obj The newly added object
		 * TODO: Get rid of the following parameter, the client itself must decide how to react to this event.
		 * 		This is a notification, not a directive.
		 * @param selectNewObject Whether the newly created object should be selected in any views watching for object addition
		 */
		void objectAdded(nap::rtti::Object& obj, bool selectNewObject);

		/**
		 * Qt Signal
		 * Invoked after an object has changed drastically
		 */
		void objectChanged(nap::rtti::Object& obj);

		/**
		 * Qt Signal
		 * Invoked just before an object is removed (including Entities)
		 * @param object The object about to be removed
		 */
		void objectRemoved(nap::rtti::Object& object);

		/**
		 * Qt Signal
		 * Invoked just after a property's value has changed
		 * @param object The object that has the changed property
		 * @param path The path to the property that has changed
		 */
		void propertyValueChanged(const PropertyPath& path);


	private:
		/**
		 * @param suggestedName
		 * @return
		 */
		std::string getUniqueName(const std::string& suggestedName);


		nap::Core& mCore;                        // nap's core
		nap::rtti::OwnedObjectList mObjects;    // The objects in this document
		QString mCurrentFilename;				// This document's filename
		QUndoStack mUndoStack;					// This document's undostack
	};

}