#include "compoundattribute.h"
#include "resourcelinkattribute.h"
#include "logger.h"

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
    
    
    ResourceLinkAttribute& CompoundAttribute::addResourceLinkAttribute(const std::string& name, const RTTI::TypeInfo& type)
    {
        ResourceLinkAttribute& link_attr = addChild<ResourceLinkAttribute>(name);
        link_attr.setResourceType(type);
        return link_attr;
    }
    
    
    AttributeBase* CompoundAttribute::addAttribute(const std::string& name, const RTTI::TypeInfo& attributeType)
    {
        if (!attributeType.isKindOf(RTTI_OF(nap::AttributeBase)))
        {
            nap::Logger::warn(*this, "can't add object: %s, not an attribute", attributeType.getName().c_str());
            return nullptr;
        }
        return &static_cast<AttributeBase&>(addChild(name, attributeType));
    }
    
    
    void CompoundAttribute::addAttribute(AttributeBase& attr)
    {
        addChild(attr);
    }
    
    
    CompoundAttribute* CompoundAttribute::getOrCreateCompoundAttribute(const std::string& name)
    {
        auto result = getChild<CompoundAttribute>(name);
        if (!result)
        {
            if (hasChild(name))
                return nullptr;
            result = &addCompoundAttribute(name);
        }
        return result;
    }
    
    
    void CompoundAttribute::removeAttribute(const std::string name)
    {
        removeChild(name);
    }
    
    
    void CompoundAttribute::removeAttribute(size_t index)
    {
        // Make sure index is in bounds
        if (index >= size())
        {
            nap::Logger::warn("unable to remove child at index: %d, index out of bounds");
            return;
        }
        
        // Fetch attr
        auto child = getAttribute(index);
        if (child == nullptr)
        {
            nap::Logger::warn("unable to query child at index: %d");
            return;
        }
        removeChild(*child);
    }
    
    
    size_t CompoundAttribute::size() const
    {
        return getNumberOfChildren();
    }
    
    
    AttributeBase* CompoundAttribute::getAttribute(const std::string& name)
    {
        return getChild<AttributeBase>(name);
    }
    
    
    // Return attribute based on index, does implicit type conversion
    AttributeBase* CompoundAttribute::getAttribute(size_t index)
    {
        nap::Object* obj = getChild(index);
        if (obj == nullptr)
            return nullptr;
        assert(obj->getTypeInfo().isKindOf(RTTI_OF(AttributeBase)));
        return static_cast<AttributeBase*>(obj);
    }
    
    
    void CompoundAttribute::childSizeChanged(CompoundAttribute& child)
    {
        sizeChanged(child);
    }
    
}

