//
//  Deform.cpp
//  Project
//
//  Created by Casimir Geelhoed on 26/08/2019.
//
//

#include "Deform.h"

// Spatial includes
#include <Spatial/Core/ParameterComponent.h>

RTTI_BEGIN_CLASS(nap::spatial::DeformBase)
RTTI_PROPERTY("Name", &nap::spatial::DeformBase::mName, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_DEFINE_BASE(nap::spatial::DeformInstance)

namespace nap
{
    namespace spatial
    {
        
        bool DeformInstance::init(nap::utility::ErrorState& errorState, nap::EntityInstance* entity)
        {
            if(entity == nullptr)
                return false;
            
            mParameterManager.init(entity->getComponent<ParameterComponentInstance>(), "deform/" + getName(), "deform/shared");
            
            onInit(entity);
            
            return true;
        };
        
    }
}
