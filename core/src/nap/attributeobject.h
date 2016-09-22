#pragma once

#include "attribute.h"
#include "object.h"
#include <assert.h>
#include <memory>
#include <mutex>
#include <nap/signalslot.h>
#include <rtti/rtti.h>
#include <unordered_map>

namespace nap
{

	template <typename T>
	class Attribute;
	class Component;

	// AttributeObject is a base class, to be implemented by classes such as Component and Entity
	//
	// Both an entity and Component are getOperators within the system. As such they carry a
	// unique identifier for easy access throughout the system. Every object can hold any
	// number of attributes.
	class AttributeObject : public Object
	{
		// AttributeBase is a friend of object, and can therefore add itself as an attribute
		RTTI_ENABLE_DERIVED_FROM(Object)
		// string:attribute map

	public:
		AttributeObject() : Object() {}
        
        // Constructor to define an AttributeObject as a class member of it's parent
        AttributeObject(Object* parent, const std::string& name);

		// Add an attribute using RTTI without templates, @type should be the type of the attribute, not the data!
        AttributeBase& addAttribute(const std::string& name, RTTI::TypeInfo type);

		// Add an attribute templated, T is the type of the atribute's data
		template <typename T>
        Attribute<T>& addAttribute(const std::string& name) { return addChild<Attribute<T>>(name); }

		// Add an attribute templated with a default value
		template <typename T>
		Attribute<T>& addAttribute(const std::string& name, const T& defaultValue)
        {
            Attribute<T>& attribute = addAttribute<T>(name);
            attribute.setValue(defaultValue);
            return attribute;
        }

		// Remove an attribute, must be a child attribute that has been added dynamically
		bool removeAttribute(AttributeBase& attribute) { return removeChild(attribute); }

		// sets the default value if the attribute is to be created, return null if attribute of different value type exists
		template <typename T>
        Attribute<T> * getOrCreateAttribute(const std::string &name, const T &defaultValue);

		// If the attribute exists, return it, otherwise create and return it. Return null if attribute of different value type exists
		template <typename T>
		Attribute<T>* getOrCreateAttribute(const std::string& name);

		// Check if this AttributeObject owns an attribute
		bool hasAttribute(const std::string& name) const;

		// Find and return an attribute by its unique name. Returns nullptr when not found.
        AttributeBase* getAttribute(const std::string& name) { return getChild<AttributeBase>(name); }

		// Retrieve attribute by type and name
		template <typename T>
		Attribute<T>* getAttribute(const std::string& name)
		{
			Attribute<T>* attribute = dynamic_cast<Attribute<T>*>(getAttribute(name));
			if (attribute) return attribute;
			return nullptr;
		}

		// Retrieve all registered attributes as string:attribute map
        std::vector<AttributeBase*> getAttributes() { return getChildrenOfType<AttributeBase>(); }

	};


	// --- template definitions ---- //

    template <typename T>
    Attribute<T>* AttributeObject::getOrCreateAttribute(const std::string& name)
    {
        auto attribute = getAttribute<T>(name);
        
        if (!attribute) {
            if (hasAttribute(name)) // Attribute of wrong type?
                return nullptr;
            
            auto type = RTTI_OF(Attribute<T>);
            
            attribute = static_cast<Attribute<T>*>(&addChild(name, type));
        }
        return attribute;
    }
    

	template <typename T>
    Attribute<T> * AttributeObject::getOrCreateAttribute(const std::string &name, const T &defaultValue)
	{
		bool valueExisted = hasAttribute(name);
		auto attribute = getOrCreateAttribute<T>(name);
		if (!valueExisted) attribute->setValue(defaultValue);
		return attribute;
	}




}

RTTI_DECLARE_BASE(nap::AttributeObject)
