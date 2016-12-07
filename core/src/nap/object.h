#pragma once

// Local Includes
#include "configure.h"
#include "signalslot.h"

// External Includes
#include <assert.h>
#include <memory>
#include <mutex>
#include <rtti/rtti.h>
#include <unordered_map>

namespace nap
{

	/**
	 * Flags to denote meta properties of an object, mainly used for the editor
	 */
	enum ObjectFlag : int {
		Visible = 1 << 0,   // Whether the user can see this object
		Editable = 1 << 1,  // Whether the user may change this object
		Removable = 1 << 2, // Whether the user may remove this object
	};

	/**
	 * The topmost base class for most NAP classes, providing hierarchical structure and inspectability which is useful
	 * for data serialization amongst other things.
	 * Instantiation should never be done "manually" but through the addChild methods. An object should always have a
	 * parent (except for the "world's" root Object, which belongs to Core)
	 */
	class Object
	{
		RTTI_ENABLE()
	public:
		// Construction / Destruction
		Object() = default;
		virtual ~Object() = default;

		/**
		 * Copy is not allowed
		 */
		Object(Object&) = delete;
		Object& operator=(const Object&) = delete;

		/**
		 * Move is not allowed
		 */
		Object(Object&&) = delete;
		Object& operator=(Object&&) = delete;

		/**
		 * @return This object's name (which is unique amongst sibling Objects)
		 */
		const std::string& getName() const;

		/**
		 * Set this Object's name, the resulting name may be different when the given name was not unique amongst its
		 * siblings or when invalid characters were provided.
		 * A signal will be dispatched, notifying observers of the change.
		 *
		 *
		 * @param name The suggested name of the object
		 * @return The Object's actual name after sanitizing it
		 */
		const std::string& setName(const std::string& name);

		/**
		 * Add a child object to this object. TODO: Needs more better documentation
         * A signal will be dispatched, notifying observers of the change.
		 * @param child The child to be added
		 */
		void addChild(Object& child);

		/**
		 * Create and add a child of type T to this object, also provide a suggested name for the new Object.
         * A signal will be dispatched, notifying observers of the change.
		 * @param name The name of the newly created Object
		 * @return The newly created Object
		 */
		template <typename T>
		T& addChild(const std::string& name)
		{
			return static_cast<T&>(addChild(name, RTTI::TypeInfo::get<T>()));
		}

        /**
         * Create and add a child of the provided type and give it the suggested name.
         * A signal will be dispatched, notifying observers of the change.
         * @param name The name of the newly created Object
         * @param type The type of the newly created Object
         * @return The newly created Object
         */
		Object& addChild(const std::string& name, const RTTI::TypeInfo& type);

		/**
		 * Given the provided type, create and add a child Object of the provided type. A default name will be given to the object.
         * A signal will be dispatched, notifying observers of the change.
		 * @param type The type of the newly created Object
		 * @return The newly created Object
		 */
		Object& addChild(const RTTI::TypeInfo& type);

		/**
		 * Add the provided child and require ownership transfer.
         * A signal will be dispatched, notifying observers of the change.
		 * @param child
		 * @return The added object
		 */
		Object& addChild(std::unique_ptr<Object> child);

		/**
		 * Remove a child
         * A signal will be dispatched, notifying observers of the change.
		 * @param child The child to remove
		 * @return true if the child was successfully removed, false otherwise
		 */

		bool removeChild(Object& child);

		/**
		 * Look up a child by name and remove it.
         * A signal will be dispatched, notifying observers of the change.
		 * @param name The name of the child that must be removed
		 * @return true if the child was successfully removed, false otherwise
		 */
		bool removeChild(const std::string& name);

		/**
		 * Remove all children and notify observers
		 */
		void clearChildren();


        /**
         * @return This object's flags. Mainly used for the editor
         */
        int getFlags() const {
            return mFlags;
        }

		/**
		 * Check if a flag has been set on this object
		 * @param flag The flag to be tested
		 * @return True if the flag was set, false otherwise
		 */
		bool checkFlag(const ObjectFlag& flag) const {
            return (mFlags & flag) != 0;
        }

		/**
		 * Set or unset a flag on this object.
		 * @param flag The flag to be set or unset
		 * @param b True if it must be set, false if it must be unset
		 */
		void setFlag(const ObjectFlag& flag, bool b)
		{
			if (b)
				mFlags |= flag;
			else
				mFlags &= ~flag;
		}

		/**
		 * Remove all children of a specific type.
		 * @param type The type of children to be removed
		 */
		void clearChildren(const RTTI::TypeInfo& type);

		/**
		 * Remove all children of a specific type T
		 */
		template <typename T>
		void clearChildren()
		{
			clearChildren(RTTI_OF(T));
		}

		/**
		 * Retrieve all children of this Object. Optionally recurse the full tree below.
		 * @param recursive If true, include all children of the children etc as well
		 * @return A new vector containing the retrieved children
		 * @deprecated Use the const version of this method instead
		 */
		std::vector<Object*> getChildren(bool recursive = false);

        /**
         * Retrieve all children of this Object. Optionally recurse the full tree below.
         * @param recursive If true, include all children of the children etc as well
         * @return A new vector containing the retrieved children
         */
		const std::vector<Object*> getChildren(bool recursive = false) const;

		/**
		 * Retrieve all children of a specific type T. Optionally recurse the full tree below.
		 * @param recursive If true, include all children of the children etc as well.
		 * @return A new vector containing the retrieved children
		 * @deprecated Use the const version of this method instead
		 */
		template <typename T>
		std::vector<T*> getChildrenOfType(bool recursive = false)
		{
			std::vector<T*> result;
			for (const auto obj : getChildren(recursive))
				if (obj->getTypeInfo().isKindOf<T>())
					result.emplace_back(static_cast<T*>(obj));
			return result;
		}


        /**
         * Retrieve all children of a specific type. Optionally recurse the full tree below.
         * @param recursive If true, include all children of the children etc as well.
         * @return A new vector containing the retrieved children
         */
		template <typename T>
		std::vector<const T*> getChildrenOfType(bool recursive = false) const
		{
			std::vector<const T*> result;
			for (const auto obj : getChildren(recursive))
				if (obj->getTypeInfo().isKindOf<T>())
					result.emplace_back(static_cast<T*>(obj));
			return result;
		}

        /**
         * Retrieve all children of a specific type T. Optionally recurse the full tree below.
         * @param recursive If true, include all children of the children etc as well.
         * @return A new vector containing the retrieved children
         */
		std::vector<Object*> getChildrenOfType(const RTTI::TypeInfo& type, bool recursive = false)
		{
			std::vector<Object*> result;
			for (const auto obj : getChildren(recursive))
				if (obj->getTypeInfo().isKindOf(type))
					result.emplace_back(obj);
			return result;
		}

		/**
		 * Get the first child of type T
		 * @return The child if found, nullptr otherwise
		 */
		template <typename T>
		T* getChildOfType()
		{
			for (auto& object : mChildren)
				if (object->getTypeInfo().isKindOf<T>())
					return static_cast<T*>(object);
			return nullptr;
		}

		// Get the first child with type @type
		Object* getChildOfType(const RTTI::TypeInfo& type);

		// Get child by name, returns nullptr if none found
		template <typename T>
		T* getChild(const std::string& name)
		{
			for (auto& child : getChildrenOfType<T>())
				if (child->getName() == name)
					return child;

			return nullptr;
		}

		template <typename T>
		bool hasChildOfType() const
		{
			for (auto& child : mChildren)
				if (child->getTypeInfo().isKindOf<T>())
					return true;
			return false;
		}

		template <typename T>
		bool hasChildOfType(const std::string& name) const
		{
			for (auto& child : mChildren)
				if (child->getTypeInfo().isKindOf<T>() && child->getName() == name)
					return true;
			return false;
		}

		// See if this object is a child of the provided parent, recursive will walk more than one level up
		bool isChildOf(const Object& parent, bool recursive = true) const;

		// Signal emitted when the name has been changed, passes its new name as argument
		Signal<const std::string&> nameChanged;

		// Signal emitted before this object is destroyed, takes the destroyed object as argument
		Signal<Object&> removed;

		// Signal emitted when this object is added, takes the added object as argument
		Signal<Object&> added;

		// Emitted after a child has been added, takes the added child object as argument
		Signal<Object&> childAdded;

		// Emitted just before a child will be removed, takes the child object to be removed as argument
		Signal<Object&> childRemoved;

		// Find a child object by name and return it, return nullptr if not found
		Object* getChild(const std::string& name);

		// Get child by index
		Object* getChild(size_t index);

		bool hasChild(const std::string& name) const;

		// Return the index of the provided child or -1 if the child wasn't found
		int getChildIndex(Object& child);

		// Return this object's parent
		Object* getParentObject() const { return mParent; }

		// Has parent
		bool hasParent() const { return mParent != nullptr; }

		// Return the root of the tree this AttributeObject sits in const.
		const Object* getRootObject() const;

		// Return the root of the tree this AttributeObject sits in non-const.
		Object* getRootObject();

	protected:
		// Return a unique name among this object's siblings
		std::string getUniqueName(const std::string& suggestedName);

		// Ensure the provided name is formatted according to the rules
		static std::string sanitizeName(const std::string& name);

		// This object's unique name
		mutable std::string mName;

		// Sets the parent
		void setParent(Object& inParent);

		// List of children owned by this object, meaning of which the base class takes ownership
		std::vector<std::unique_ptr<Object>> mOwnedObjects;

		// List of children of this object
		std::vector<Object*> mChildren;

	private:
		// This object's parent
		Object* mParent = nullptr;
		int mFlags = Visible | Editable | Removable;
	};
}

RTTI_DECLARE_BASE(nap::Object)
