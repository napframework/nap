#include "SpatialTransformationComponent.h"


// Spatial includes
#include <Spatial/Core/ExposedDataComponent.h>

// Nap includes
#include <entity.h>

// RTTI
RTTI_BEGIN_CLASS(nap::spatial::SpatialTransformationComponent)
RTTI_PROPERTY("SoundObjectTransformationChain", &nap::spatial::SpatialTransformationComponent::mSoundObjectTransformationChain, nap::rtti::EPropertyMetaData::Required);
RTTI_PROPERTY("ShapeComponent", &nap::spatial::SpatialTransformationComponent::mShapeComponent, nap::rtti::EPropertyMetaData::Required);
RTTI_PROPERTY("ParticleTransformationChain", &nap::spatial::SpatialTransformationComponent::mParticleTransformationChain, nap::rtti::EPropertyMetaData::Default);
RTTI_END_CLASS


RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::spatial::SpatialTransformationComponentInstance)
    RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

namespace nap
{
    namespace spatial
    {
        
        
        void SpatialTransformationComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
        {
            components.push_back(RTTI_OF(ExposedDataComponent));
        }

        
        SpatialTransformationComponentInstance::SpatialTransformationComponentInstance(EntityInstance& entity, Component& resource) : ComponentInstance(entity, resource)
        {
        }

        
        
        bool SpatialTransformationComponentInstance::init(utility::ErrorState& errorState)
        {
            
            auto& exposedData = getEntityInstance()->getComponent<ExposedDataComponentInstance>().getRoot();
            exposedData.expose<glm::vec3>("soundObjectDimensions", [&](){ return getSoundObjectTransform().mScale; });
            exposedData.expose<glm::vec3>("soundObjectPosition", [&](){ return getSoundObjectTransform().mPosition; });
            exposedData.expose<glm::vec4>("soundObjectRotation", [&](){ return getSoundObjectTransform().mRotation; });

            return true;
        }
        
        void SpatialTransformationComponentInstance::update(double deltaTime)
        {
            
            // 1. Get SoundObjectTransform.
            mSoundObjectTransform.mPosition = glm::vec3(0.);
            mSoundObjectTransform.mRotation = glm::vec4(0., 0., 1., 0.);
            mSoundObjectTransform.mScale = glm::vec3(1.);
            
            mSoundObjectTransformationChain->apply(mSoundObjectTransform);
            
            // 2. Get world space particle transforms based on the sound object transform.
            mWorldSpaceParticleTransforms = mShapeComponent->calculateParticleTransforms(mSoundObjectTransform);
            
            // 3. Execute particle level transformationchain (with dimensions, spatial dynamics, particle-level globals)
            // TODO optimisation: could/should SpatialTransformComponent know about the limit of channels of SpatialAudioComponent to not calculate unused particles?
            for(int i = 0; i < mWorldSpaceParticleTransforms.size(); i++)
            {
                if(mParticleTransformationChain != nullptr)
                    mParticleTransformationChain->apply(mWorldSpaceParticleTransforms[i]);
            }
            
            
        }
        
    }
}
