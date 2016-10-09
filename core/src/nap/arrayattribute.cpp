//
//  ArrayAttribute.cpp
//  Soundlab
//
//  Created by Stijn van Beek on 10/3/16.
//
//

#include "arrayattribute.h"

RTTI_DEFINE(nap::CompoundAttribute)

namespace nap {
    
    
    CompoundAttribute::CompoundAttribute()
    {
        initialize();
    }
    
    
    CompoundAttribute::CompoundAttribute(AttributeObject* parent, const std::string& name, bool atomic) : AttributeBase(parent, name, atomic)
    {
        initialize();
    }
    
    
    void CompoundAttribute::initialize()
    {
        childAdded.connect([&](Object& object){
            if (object.getTypeInfo().isKindOf<AttributeBase>())
            {
                auto& attribute = static_cast<AttributeBase&>(object);
                attribute.valueChanged.connect(valueChanged);
                
                if (object.getTypeInfo().isKindOf<CompoundAttribute>())
                {
                    auto& compound = static_cast<CompoundAttribute&>(object);
                    compound.sizeChanged.connect(childSizeChangedSlot);
                }
                
                sizeChanged(*this);
            }
        });
        
        childRemoved.connect([&](Object& object){
            if (object.getTypeInfo().isKindOf<AttributeBase>())
            {
                auto& attribute = static_cast<AttributeBase&>(object);
                attribute.valueChanged.disconnect(valueChanged);
                
                if (object.getTypeInfo().isKindOf<CompoundAttribute>())
                {
                    auto& compound = static_cast<CompoundAttribute&>(object);
                    compound.sizeChanged.disconnect(childSizeChangedSlot);
                }
                
                sizeChanged(*this);
            }
        });
        
    }
    
    
    // Populate @inAttribute with contents of this
    void CompoundAttribute::getValue(AttributeBase& inAttribute) const
    {
        assert(inAttribute.getTypeInfo().isKindOf(getTypeInfo()));
        
        auto& other = static_cast<CompoundAttribute&>(inAttribute);
        other.clear();
        
        for (auto& child : getAttributes())
        {
            auto& newAttribute = other.addChild(child->getName(), child->getTypeInfo());
            static_cast<AttributeBase&>(newAttribute).setValue(*child);
        }
    }
    
    
    // Populate this with contents of @inAttribute
    void CompoundAttribute::setValue(const AttributeBase& inAttribute)
    {
        assert(inAttribute.getTypeInfo().isKindOf(getTypeInfo()));
        
        auto& other = static_cast<const CompoundAttribute&>(inAttribute);
        clear();
        
        for (auto& child : other.getAttributes())
        {
            auto& newAttribute = addChild(child->getName(), child->getTypeInfo());
            static_cast<AttributeBase&>(newAttribute).setValue(*child);
        }
    }
    
    
    CompoundAttribute& CompoundAttribute::addCompoundAttribute(const std::string& name)
    {
        return addChild<CompoundAttribute>(name);
    }
    
    
    void CompoundAttribute::removeAttribute(const std::string name)
    {
        removeChild(name);
    }
    
    
    void CompoundAttribute::removeAttribute(size_t index)
    {
        auto child = getAttribute(index);
        if (child)
            removeChild(*child);
    }
    
    
    AttributeBase* CompoundAttribute::getAttribute(const std::string& name)
    {
        return getChild<AttributeBase>(name);
    }
    
    
    AttributeBase* CompoundAttribute::getAttribute(size_t index)
    {
        if (index >= size())
            return nullptr;
        return getAttributes()[index];
    }
    
    
    void CompoundAttribute::childSizeChanged(CompoundAttribute& child)
    {
        sizeChanged(child);
    }
    
    
}
