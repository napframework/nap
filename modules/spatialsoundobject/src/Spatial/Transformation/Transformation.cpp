#include "Transformation.h"

// Spatial includes
#include <Spatial/Core/ParameterComponent.h>
#include <Spatial/Utility/ParameterTypes.h>

// NAP includes
#include <entity.h>


RTTI_BEGIN_CLASS(nap::spatial::TransformationBase)
RTTI_PROPERTY("Name", &nap::spatial::TransformationBase::mName, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::spatial::TransformationInstance)
RTTI_END_CLASS

namespace nap
{
    namespace spatial
    {
        

        bool TransformationInstance::init(nap::utility::ErrorState& errorState, nap::EntityInstance* entity, ParameterVec3& anchorPoint, const std::string& transformationChainName)
        {
            if(entity == nullptr)
                return false;
            
            // init parameter manager
            mParameterManager.init(entity->getComponent<ParameterComponentInstance>(), transformationChainName + "/" + getName(), "");
            
            // set anchor point parameter pointer
            mAnchorPoint = &anchorPoint;
            
            onInit(entity);
            
            return true;
        };
        
        const glm::vec3& TransformationInstance::getAnchorPoint()
        {
            return mAnchorPoint->mValue;
        }
        
    }
}
