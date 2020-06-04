#include "Shape.h"

// Spatial includes
#include <Spatial/Core/ParameterComponent.h>

RTTI_BEGIN_CLASS(nap::spatial::ShapeBase)
    RTTI_PROPERTY("Name", &nap::spatial::ShapeBase::mName, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_DEFINE_BASE(nap::spatial::ShapeInstance)


namespace nap
{
    namespace spatial
    {
        
        bool ShapeInstance::init(nap::utility::ErrorState& errorState, nap::EntityInstance* entity)
        {
            if(entity == nullptr)
                return false;
            
            mParameterManager.init(entity->getComponent<ParameterComponentInstance>(), "shape/" + getName(), "shape/shared");
            
            onInit(entity);
            
            return true;
        };
                
    }
}
