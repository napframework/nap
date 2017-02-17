#pragma once

#include "attribute.h"
#include "arrayattribute.h"

namespace nap {
    
    // Forward declares
    class ResourceLinkAttribute;
    
    // An attribute that can hold collections of anything that is derived from @AttributeBase
    class CompoundAttribute : public AttributeBase
    {
        RTTI_ENABLE_DERIVED_FROM(AttributeBase)
        
    public:
        // Default constructor
        CompoundAttribute();
        // Constructor that takes a parent and a name
        CompoundAttribute(AttributeObject* parent, const std::string& name, bool atomic = false);
        
        // Inits slots to respond to child value changes
        void initialize();
        
        // Compound attribute has no value type because it can hold anything
        const RTTI::TypeInfo getValueType() const override { return RTTI::TypeInfo::empty(); }
        
        // Value accessors
        void getValue(AttributeBase& inAttribute) const override;
        
        // Set attribute to value
        void setValue(const AttributeBase &inAttribute) override;
        
        // Add another nested compound attribute
        CompoundAttribute& addCompoundAttribute(const std::string& name = "");
        
        // Add an array attribute holding values of type T
        template <typename T>
        ArrayAttribute<T>& addArrayAttribute(const std::string& name = "");
        
        // Add resource link attribute of resource type
        ResourceLinkAttribute& addResourceLinkAttribute(const std::string& name, const RTTI::TypeInfo& type);
        
        // Add an attribute of type T
        template <typename T>
        Attribute<T>& addAttribute(const T& value);
        
        // Add an attribute of type T
        template <typename T>
        Attribute<T>& addAttribute(const std::string& name, const T& value);
        
        // Adds an attribute of type @type, with name: @name
        AttributeBase* addAttribute(const std::string& name, const RTTI::TypeInfo& attributeType);
        
        // Add a floating attribute, this object takes ownership of the attribute
        void addAttribute(AttributeBase& attr);
        
        // Create a compound attribute if it doesn't exist and return it
        CompoundAttribute* getOrCreateCompoundAttribute(const std::string& name);
        
        // Create an array attribute if it doesn't exist and return it
        template <typename T>
        ArrayAttribute<T>* getOrCreateArrayAttribute(const std::string& name);
        
        // Create an attribute if it doesn't exist, and return it
        template <typename T>
        Attribute<T>* getOrCreateAttribute(const std::string& name);
        
        // Create an attribute if it doesn't exist, assign a value, and return it
        template <typename T>
        Attribute<T>* getOrCreateAttribute(const std::string& name, const T& defaultValue);
        
        // Get the attribute based on name if it exists
        template <typename T>
        Attribute<T>* getAttribute(const std::string& name);
        
        // Remove attribute by name from the compound
        void removeAttribute(const std::string name);
        
        // Remove an attribute by it's index
        void removeAttribute(size_t index);
        
        // Clear the compound of all of it's contents
        void clear()																		{ clearChildren(); }
        
        // Return number of attributes within the compound
        size_t size() const;
        
        // Return all attributes within the compound
        // TODO: This call is in essence slow because of the multi bound checks and RTTI
        // Can't we make sure all children are of type AttributeBase?
        std::vector<AttributeBase*> getAttributes()											{ return getChildrenOfType<AttributeBase>(); }
        
        // Return all attributes within the compound const
        std::vector<const AttributeBase*> getAttributes() const								{ return getChildrenOfType<AttributeBase>(); }
        
        // Return a sub-attribute by name
        AttributeBase* getAttribute(const std::string& name);
        
        // Return a sub-attribute by index
        AttributeBase* getAttribute(size_t index);
        
        Signal<CompoundAttribute&> sizeChanged;
        
    private:
        void childSizeChanged(CompoundAttribute& child);
        
        // Called when number of attributes changes
        Slot<CompoundAttribute&> childSizeChangedSlot = { this, &CompoundAttribute::childSizeChanged };
    };
    
    
    //////////////////////////////////////////////////////////////////////////
    // Template Definitions
    //////////////////////////////////////////////////////////////////////////
    
    
    template <typename T>
    ArrayAttribute<T>& CompoundAttribute::addArrayAttribute(const std::string& name)
    {
        return addChild<ArrayAttribute<T>>(name);
    }
    
    
    template <typename T>
    Attribute<T>& CompoundAttribute::addAttribute(const std::string& name, const T& value)
    {
        auto& attribute = addChild<Attribute<T>>(name);
        attribute.setValue(value);
        return attribute;
    }
    
    
    template <typename T>
    Attribute<T>& CompoundAttribute::addAttribute(const T& value)
    {
        auto& attribute = addChild<Attribute<T>>("");
        attribute.setValue(value);
        return attribute;
    }
    
    
    template <typename T>
    ArrayAttribute<T>* CompoundAttribute::getOrCreateArrayAttribute(const std::string& name)
    {
        auto result = getChild<ArrayAttribute<T>>(name);
        if (!result)
        {
            if (hasChild(name))
                return nullptr;
            result = &addArrayAttribute<T>(name);
        }
        return result;
    }
    
    
    template <typename T>
    Attribute<T>* CompoundAttribute::getOrCreateAttribute(const std::string& name)
    {
        auto result = getChild<Attribute<T>>(name);
        if (!result)
        {
            if (hasChild(name))
                return nullptr;
            result = &addChild<Attribute<T>>(name);
        }
        return result;
    }
    
    
    template <typename T>
    Attribute<T>* CompoundAttribute::getOrCreateAttribute(const std::string& name, const T& defaultValue)
    {
        auto result = getAttribute<T>(name);
        if (!result)
        {
            if (hasChild(name))
            {
                return nullptr;
            }
            result = &addAttribute<T>(name);
            result->setValue(defaultValue);
        }
        return result;
    }
    
    
    template <typename T>
    Attribute<T>* CompoundAttribute::getAttribute(const std::string& name)
    {
        return getChild<Attribute<T>>(name);
    }
    
    
    
}

RTTI_DECLARE(nap::CompoundAttribute)