#include "SpatialDynamicsTransformation.h"

// Spatial includes
#include <Spatial/SpatialAudio/MixdownComponent.h>
#include <Spatial/Core/ParameterComponent.h>

// NAP includes
#include <entity.h> // for EntityInstance


RTTI_DEFINE_CLASS(nap::spatial::SpatialDynamicsTransformation)

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::spatial::SpatialDynamicsTransformationInstance)
RTTI_END_CLASS

namespace nap
{
    namespace spatial
    {
        
        void SpatialDynamicsTransformationInstance::onInit(nap::EntityInstance* entity)
        {
            
            mMixdownComponent = &entity->getComponent<MixdownComponentInstance>();
            mSpatialDynamicsDimensionsAmount = getParameterManager().addParameterFloat("amount", 0., -1., 1.);
            
        }
        
        void SpatialDynamicsTransformationInstance::apply(glm::vec3& position, glm::vec3& scale, glm::vec4& rotation)
        {
            scale *= 1. + mMixdownComponent->getEnvelopeFollowerOutput() * mSpatialDynamicsDimensionsAmount->mValue;
        }
        
    }
}
