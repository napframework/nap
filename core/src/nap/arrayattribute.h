//
//  ArrayAttribute.hpp
//  Soundlab
//
//  Created by Stijn van Beek on 10/3/16.
//
//

#pragma once
#include <stdio.h>
#include <nap/attribute.h>

namespace nap {
    
    template <typename T>
    class ArrayAttribute;
    
    
    // An attribute that can hold collections of anything that is derived from @AttributeBase
    class CompoundAttribute : public AttributeBase {
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
        void setValue(const AttributeBase &inAttribute) override;
        
        // Add another nested compound attribute
        CompoundAttribute& addCompoundAttribute(const std::string& name = "");
        
        // Add an array attribute holding values of type T
        template <typename T>
        ArrayAttribute<T>& addArrayAttribute(const std::string& name = "");
        
        // Add an attribute of type T
        template <typename T>
        Attribute<T>& addAttribute(const T& value);
        
        // Add an attribute of type T
        template <typename T>
        Attribute<T>& addAttribute(const std::string& name, const T& value);
        
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
        
        // Remove attribute by name from the compound
        void removeAttribute(const std::string name);
        // Remove an attribute by it's index
        void removeAttribute(size_t index);
        
        // Clear the compound of all of it's contents
        void clear() { clearChildren(); }
        
        // Return number of attributes within the compound
        size_t size() { return getAttributes().size(); }
        
        // Return all attributes within the compound
        std::vector<AttributeBase*> getAttributes() { return getChildrenOfType<AttributeBase>(); }
        // Return all attributes within the compound const
        std::vector<const AttributeBase*> getAttributes() const { return getChildrenOfType<AttributeBase>(); }
        // Return a sub-attribute by name
        AttributeBase* getAttribute(const std::string& name);
        // Return a sub-attribute by index
        AttributeBase* getAttribute(size_t index);
        
        Signal<CompoundAttribute&> sizeChanged;
        
    protected:
        Link& getLink() const override { return mLink; }
        
    private:
        void childSizeChanged(CompoundAttribute& child);
        
        // Inherited from BaseAttribute, so that BaseAttribute can trigger the valueChanged() signal to be emitted
        // TODO redundant?
        void emitValueChanged() override final { }
        
        mutable TypedLink<CompoundAttribute> mLink = { *this };
        
        Slot<CompoundAttribute&> childSizeChangedSlot = { this, &CompoundAttribute::childSizeChanged };
    };
    
    
    // An attribute that holds a collection of attributes of one specific datatype T
    template <typename T>
    class ArrayAttribute : public CompoundAttribute {
        RTTI_ENABLE_DERIVED_FROM(CompoundAttribute)
        
    public:
        // Default constructor
        ArrayAttribute() = default;
        // Constructor that takes the attribute's parent and name
        ArrayAttribute(AttributeObject* parent, const std::string& name, bool atomic = false) : CompoundAttribute(parent, name, atomic) {}
        
        // Returns the type of the values within the array
        const RTTI::TypeInfo getValueType() const override;
        
        // Add an attribute with @value
        Attribute<T>& addAttribute(const T& value);
        
        // Add an attribute with @name for mapping and @value
        Attribute<T>& addAttribute(const std::string& name, const T& value);
        
        // Value accessors
        std::vector<T> getValues();
        void setValues(const std::vector<T>& values);
        
        // subscript operator implementations
        T& operator[](size_t index) { return getAttributes()[index]->getValueRef(); }
        const T& operator[](size_t index) const { return getAttributes()[index]->getValue(); }
        T& operator[](const std::string& name) { return getChild<Attribute<T>>(name)->getValueRef(); }
        const T& operator[](const std::string& name) const { return getChild<Attribute<T>>(name)->getValue(); }
        
        // Return the attributes within the array
        std::vector<Attribute<T>*> getAttributes() { return getChildrenOfType<Attribute<T>>(); }
//        const std::vector<Attribute<T>*> getAttributes() const { return getChildrenOfType<Attribute<T>>(); }

        // Return an attribute within the array by name
        Attribute<T>* getAttribute(const std::string& name);

        // Return an attribute within the array by index
        Attribute<T>* getAttribute(size_t index);
        
    protected:
        Link& getLink() const override { return mLink; }
        
    private:
        mutable TypedLink<ArrayAttribute<T>> mLink = { *this };
        
    };
    
    
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
        auto result = getChild<Attribute<T>>(name);
        if (!result)
        {
            if (hasChild(name))
                return nullptr;
            result = &addAttribute<T>(name);
            result->setValue(defaultValue);
        }
        return result;
    }
    
    
    // Templated definition of getValueType
    template <typename T>
    const RTTI::TypeInfo ArrayAttribute<T>::getValueType() const
    {
        return RTTI::TypeInfo::get<T>();
    }
    
    
    template <typename T>
    std::vector<T> ArrayAttribute<T>::getValues()
    {
        std::vector<T> result;
        for (auto& attribute : getAttributes())
            result.emplace_back(attribute->getValue());
        return result;
    }
    
    
    template <typename T>
    void ArrayAttribute<T>::setValues(const std::vector<T>& values)
    {
        clear();
        for (auto& value : values)
            addAttribute(value);
    }
    
    
    template <typename T>
    Attribute<T>& ArrayAttribute<T>::addAttribute(const T& value)
    {
        auto& child = addChild<Attribute<T>>("");
        child.setValue(value);
        return child;
    }
    
    
    template <typename T>
    Attribute<T>& ArrayAttribute<T>::addAttribute(const std::string& name, const T& value)
    {
        auto& child = addChild<Attribute<T>>(name);
        child.setValue(value);
        return child;
    }
    
    
    template <typename T>
    Attribute<T>* ArrayAttribute<T>::getAttribute(const std::string& name)
    {
        return getChild<Attribute<T>>(name);
    }

    
    template <typename T>
    Attribute<T>* ArrayAttribute<T>::getAttribute(size_t index)
    {
        if (index >= size())
            return nullptr;
        return getAttributes()[index];
    }
    

}

RTTI_DECLARE(nap::CompoundAttribute)
