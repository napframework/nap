//
//  ArrayAttribute.cpp
//  Soundlab
//
//  Created by Stijn van Beek on 10/3/16.
//
//

#include "ArrayAttribute.h"

RTTI_DEFINE(nap::CompoundAttribute)

namespace nap {
    
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
    
    
    
}
