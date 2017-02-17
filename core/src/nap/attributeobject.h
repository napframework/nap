#pragma once

// Local Includes
#include "attribute.h"
#include "arrayattribute.h"
#include "compoundattribute.h"
#include "object.h"
#include "signalslot.h"
#include "logger.h"

// External Includes
#include <assert.h>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <rtti/rtti.h>

namespace nap
{

	/**
	 * An attribute object can hold one or more attributes. Each attribute is an Object as well and as such will be identified by a unique name.
	 */
	class AttributeObject : public Object
	{
		RTTI_ENABLE_DERIVED_FROM(Object)

	public:
		AttributeObject() : Object() {}
        
        /**
         * WARNING: Do not use this in client code.
         * Constructing using this method may put Object indexing using unique names in an inconsistent state1
         * Use Object::addChild instead
         *
         * Create an attribute under the specified parent.
         * @param parent The parent of this AttributeObject
         * @param name The name of
         */
        AttributeObject(Object* parent, const std::string& name);

		/**
		 * Add an attribute with the specified value type
		 * @param name The proposed name of this attribute,
		 * the system may decide to use another name ir order to avoid collision
		 * @param attributeType The type of the attribute, not it's value
		 * @return The newly created Attribute
		 */
        AttributeBase& addAttribute(const std::string &name, RTTI::TypeInfo attributeType);

		/**
		 * Add an attribute with the specified name
		 * @tparam T The value type of the newly created Attribute
		 * @param name The proposed name of this attribute,
		 * the system may decide to use another name ir order to avoid collision
		 * @return The newly created Attribute
		 */
		template <typename T>
        Attribute<T>& addAttribute(const std::string& name) { return addChild<Attribute<T>>(name); }

		/**
		 * Add an Attribute with the specified value type and name
		 * @tparam T The value type of the Attribute to be created
		 * @param name The name of the attribute to be created
		 * @param defaultValue This will be the value of the Attribute
		 * @return The newly created Attribute
		 */
		template <typename T>
		Attribute<T>& addAttribute(const std::string& name, const T& defaultValue);
        
        /**
		 * Add an attribute that will itself hold more attributes.
		 * @param name The name of the CompoundAttribute to be created
		 * @return The newly created CompoundAttribute.
		 */
        CompoundAttribute& addCompoundAttribute(const std::string& name) { return addChild<CompoundAttribute>(name); }

		/**
		 * Add an array attribute holding attributes of type T
		 * @tparam T The element value type of the attribute
		 * @param name The name of the array attribute to be created
		 * @return The newly created array attribute
		 */
        template <typename T>
        ArrayAttribute<T>& addArrayAttribute(const std::string& name) { return addChild<ArrayAttribute<T>>(name); }

		/**
		 * Remove and destroy the provided Attribute.
		 * @param attribute The attribute to be removed.
		 * @return True if removal was successful, false otherwise.
		 */
		bool removeAttribute(AttributeBase& attribute) { return removeChild(attribute); }

		/**
		 * Retrieve an Attribute, or create it if no Attribute with the specified name was found.
		 * @tparam T The value type of the Attribute
		 * @param name The name of the Attribute to be created or retrieved
		 * @param defaultValue On creation, set this value on the new Attribute
		 * @return An Attribute or nullptr when creation failed
		 */
		template <typename T>
        Attribute<T> * getOrCreateAttribute(const std::string &name, const T &defaultValue);

		/**
		 * Retrieve an Attribute, or create it if no Attribute with the specified name was found.
		 * @tparam T The value type of the Attribute
		 * @param name The name of the Attribute to be created or retrieved
		 * @return An Attribute or nullptr when creation failed
		 */
		template <typename T>
		Attribute<T>* getOrCreateAttribute(const std::string& name);

		/**
		 * Retrieve an attribute with the specified name and type, if it doesn't exist, create one and return that.
		 * @param name The name of the attribute to be found or given to the newly created one
		 * @param valueType The value type of the attribute to be found or created
		 * @return The retrieved or created attribute, nullptr if the provided type was different from the existing attribute.
		 */
		AttributeBase* getOrCreateAttribute(const std::string& name, const RTTI::TypeInfo& valueType);

		/**
		 * Check if this object has an attribute with the provided name
		 * @param name The name of the attribute to look for
		 * @return True if an attribute with the provided name exists, false otherwise
		 */
		bool hasAttribute(const std::string& name) const;

		/**
		 * Retrieve an attribute with the specified name
		 * @param name The name of the attribute to get
		 * @return The attribute with the specified name or nullptr when no such attribute has been found
		 */
        AttributeBase* getAttribute(const std::string& name) { return getChild<AttributeBase>(name); }

		/**
		 * Retrieve an attribute by name and type T
		 * @tparam T The expected value type of the attribute
		 * @param name The exact name of the attribute
		 * @return The attribute if it was found, nullptr otherwise
		 */
		template <typename T>
		Attribute<T>* getAttribute(const std::string& name);

		/**
		 * @return All of this Object's Attribute instances
		 */
        std::vector<AttributeBase*> getAttributes() { return getChildrenOfType<AttributeBase>(); }

		/**
		 * @return All of this Object's Attribute instances that have value type T
		 */
        template <typename T>
        std::vector<Attribute<T>*> getAttributesOfType() { return getChildrenOfType<Attribute<T>>(); }

	};


}

RTTI_DECLARE(nap::AttributeObject)

#include "attributeobject.hpp"
