#pragma once

#include <assert.h>
#include <memory>
#include <mutex>
#include <nap/signalslot.h>
#include <rtti/rtti.h>
#include <unordered_map>

namespace nap
{
    
	class Object
	{
		RTTI_ENABLE()
	public:
		// Construction / Destruction
		Object() = default;
        virtual ~Object() = default;

		// Default copy behaviour
		Object(Object&) = default;
		Object& operator=(const Object&) = default;

		const std::string& getName() const;

		const std::string & setName(const std::string &name);
        
        void addChild(Object& child);

		template <typename T>
		T& addChild(const std::string& name)
		{
			return static_cast<T&>(addChild(name, RTTI::TypeInfo::get<T>()));
		}

		Object& addChild(const std::string& name, const RTTI::TypeInfo& type);

		Object& addChild(const RTTI::TypeInfo& type);

		Object& addChild(std::unique_ptr<Object> child);

		// Remove a child, return true if child was found and removed, false otherwise
		bool removeChild(Object& child);

		// Remove a child by name, return true if child was found and removed, false otherwise
		bool removeChild(const std::string& name);

		// Clears all children
        void clearChildren();

        // Clears all children of a specific type
		void clearChildren(const RTTI::TypeInfo& inInfo);

        // Clears all children of a specific type T
		template <typename T>
		void clearChildren()
		{
			clearChildren(RTTI_OF(T));
		}

		// Returns all children
		std::vector<Object*> getChildren(bool recursive = false);

		// Retrieve children of this object filtered by template type
		template <typename T>
		std::vector<T*> getChildrenOfType(bool recursive = false)
		{
            std::vector<T*> result;
			for (const auto obj : getChildren(recursive))
				if (obj->getTypeInfo().isKindOf<T>()) result.emplace_back(static_cast<T*>(obj));
            return result;
		}

        
        // Retrieve children of this object filtered by RTTI type
        std::vector<Object*> getChildrenOfType(const RTTI::TypeInfo& type, bool recursive = false) {
            std::vector<Object*> result;
            for (const auto obj : getChildren(recursive))
                if (obj->getTypeInfo().isKindOf(type)) result.emplace_back(obj);
            return result;

        }

        // Get the first child of type T, returns nullptr if none found
		template <typename T>
		T* getChildOfType()
		{
            for (auto& object : mChildren)
                if (object->getTypeInfo().isKindOf<T>()) return static_cast<T*>(object);
            return nullptr;
		}

		// Get the first child with type @type
		Object* getChildOfType(const RTTI::TypeInfo& type);

		// Get child by name, returns nullptr if none found
		template <typename T>
		T* getChild(const std::string& name)
		{
			for (auto& child : getChildrenOfType<T>())
				if (child->getName() == name) return child;

			return nullptr;
		}

		template <typename T>
		bool hasChildOfType() const
		{
			for (auto& child : mChildren)
				if (child->getTypeInfo().isKindOf<T>()) return true;
			return false;
		}

        template <typename T>
        bool hasChildOfType(const std::string& name) const
        {
            for (auto& child : mChildren)
                if (child->getTypeInfo().isKindOf<T>() && child->getName() == name) return true;
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
		Object* getChild(int index);
        
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
	};
}

RTTI_DECLARE_BASE(nap::Object)
