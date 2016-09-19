#include "attributeobject.h"
#include <algorithm>
#include <nap/component.h>


RTTI_DEFINE(nap::AttributeObject)

namespace nap
{

    AttributeBase& AttributeObject::addAttribute(const std::string& name, RTTI::TypeInfo type)
    {
        assert(type.isKindOf<AttributeBase>());
        return *static_cast<AttributeBase*>(&addChild(name, type));
    }

    
    bool AttributeObject::hasAttribute(const std::string& name) const
    {
        return hasChildOfType<AttributeBase>(name);
    }
    
}
