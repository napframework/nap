#include "___FILEBASENAME___.h"

// Nap includes
#include <entity.h>

// RTTI
RTTI_BEGIN_CLASS(nap::___VARIABLE_className___)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::___VARIABLE_className___Instance)
    RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

namespace nap
{
    
    bool ___VARIABLE_className___Instance::init(utility::ErrorState& errorState)
    {
        
    }
        
}
