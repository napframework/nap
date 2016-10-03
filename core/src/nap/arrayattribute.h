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
    
    class CompoundAttribute : public AttributeBase {
        RTTI_ENABLE_DERIVED_FROM(AttributeBase)
        
    public:
        CompoundAttribute() = default;
        CompoundAttribute(AttributeObject* parent, const std::string& name, bool atomic = false) : AttributeBase(parent, name, atomic) {}
        
        const RTTI::TypeInfo getValueType() const override { return RTTI::TypeInfo::empty(); }
        
        // Value accessors
        void getValue(AttributeBase& inAttribute) const override;
        void setValue(const AttributeBase &inAttribute) override;
        
        CompoundAttribute& addCompoundAttribute(const std::string& name = "");
        
        template <typename T>
        ArrayAttribute<T>& addArrayAttribute(const std::string& name = "");
        
        template <typename T>
        Attribute<T>& addAttribute(const T& value, const std::string& name = "");
        
        void removeAttribute(const std::string name);
        void removeAttribute(size_t index);
        
        void clear() { clearChildren(); }
        
        size_t size() { return getAttributes().size(); }
        
        std::vector<AttributeBase*> getAttributes() { return getChildrenOfType<AttributeBase>(); }
        std::vector<const AttributeBase*> getAttributes() const { return getChildrenOfType<AttributeBase>(); }
        AttributeBase* getAttribute(const std::string& name);
        AttributeBase* getAttribute(size_t index);
        
    protected:
        Link& getLink() const override { return mLink; }
        
    private:
        // Inherited from BaseAttribute, so that BaseAttribute can trigger the valueChanged() signal to be emitted
        // TODO redundant?
        void emitValueChanged() override final { }
        
        mutable TypedLink<CompoundAttribute> mLink = { *this };
        
    };
    
    
    template <typename T>
    class ArrayAttribute : public CompoundAttribute {
        RTTI_ENABLE_DERIVED_FROM(CompoundAttribute)
        
    public:
        ArrayAttribute() = default;
        ArrayAttribute(AttributeObject* parent, const std::string& name, bool atomic = false) : CompoundAttribute(parent, name, atomic) {}
        
        const RTTI::TypeInfo getValueType() const override;
        
        Attribute<T>& addAttribute(const T& value, const std::string& name = "");
        
        T& operator[](size_t index) { return getAttributes()[index]->getValueRef(); }
        const T& operator[](size_t index) const { return getAttributes()[index]->getValue(); }
        T& operator[](const std::string& name) { return getChild<Attribute<T>>(name)->getValueRef(); }
        const T& operator[](const std::string& name) const { return getChild<Attribute<T>>(name)->getValue(); }
        
        std::vector<Attribute<T>*> getAttributes() { return getChildrenOfType<Attribute<T>>(); }
        const std::vector<Attribute<T>*> getAttributes() const { return getChildrenOfType<Attribute<T>>(); }
        Attribute<T>* getAttribute(const std::string& name);
        Attribute<T>* getAttribute(size_t index);
        
    protected:
        Link& getLink() const override { return mLink; }
        
    private:
        mutable TypedLink<ArrayAttribute<T>> mLink = { *this };
        
    };
    
    
    template <typename T>
    ArrayAttribute<T>& CompoundAttribute::addArrayAttribute(const std::string& name)
    {
        auto& attribute = addChild<ArrayAttribute<T>>(name);
        return attribute;
    }
    
    
    template <typename T>
    Attribute<T>& CompoundAttribute::addAttribute(const T& value, const std::string& name)
    {
        auto& attribute = addChild<Attribute<T>>(name);
        attribute.setValue(value);
        return attribute;
    }
    
    
    // Templated definition of getValueType
    template <typename T>
    const RTTI::TypeInfo ArrayAttribute<T>::getValueType() const
    {
        return RTTI::TypeInfo::get<T>();
    }
    
    
    template <typename T>
    Attribute<T>& ArrayAttribute<T>::addAttribute(const T& value, const std::string& name)
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
