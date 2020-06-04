#include "TransformationChainComponent.h"

// Spatial includes
#include <Spatial/Core/ParameterComponent.h>
#include <Spatial/Transformation/ExternalTransformation.h>
#include <Spatial/SpatialAudio/MixdownComponent.h>

// Nap includes
#include <entity.h>

// RTTI
RTTI_BEGIN_CLASS(nap::spatial::TransformationChainComponent)
    RTTI_PROPERTY("Transformations", &nap::spatial::TransformationChainComponent::mTransformations, nap::rtti::EPropertyMetaData::Embedded)
    RTTI_PROPERTY("Name", &nap::spatial::TransformationChainComponent::mName, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::spatial::TransformationChainComponentInstance)
    RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
    RTTI_FUNCTION("addExternalTransformation", &nap::spatial::TransformationChainComponentInstance::addExternalTransformation)
    RTTI_FUNCTION("addSwitchExternalTransformation", &nap::spatial::TransformationChainComponentInstance::addSwitchExternalTransformation)
RTTI_END_CLASS

namespace nap
{
    namespace spatial
    {
        
        
        void TransformationChainComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
        {
            components.push_back(RTTI_OF(ParameterComponent));
            components.push_back(RTTI_OF(MixdownComponent)); // for spatial dynamics transformation
            // IDEA: give the Transformation class a getDependentComponents()-method and loop through all of them from here.
        }

        
        
        TransformationChainComponentInstance::TransformationChainComponentInstance(EntityInstance& entity, Component& resource) : ComponentInstance(entity, resource) { }

        
        
        bool TransformationChainComponentInstance::init(utility::ErrorState& errorState)
        {
            // Create the shared 'anchorpoint' parameter before instantiating the transformation instances, if the parameter doesn't exist already.
            // There is one anchor point per entity, that can be requested by TransformationInstances in all transformationchaincomponents.
            auto& parameterComponent = getEntityInstance()->getComponent<ParameterComponentInstance>();
            mAnchorPoint = parameterComponent.findParameter<ParameterVec3>("anchorPoint");
            if(mAnchorPoint == nullptr)
                mAnchorPoint = &parameterComponent.addParameterVec3("anchorPoint", glm::vec3(0.0), -9999, 9999);
            
            mName = getComponent<TransformationChainComponent>()->mName;
            
            // instantiate transformation instances
            for(auto& transformationResource : getComponent<TransformationChainComponent>()->mTransformations)
                mTransformations.push_back(transformationResource->instantiate(errorState, getEntityInstance(), *mAnchorPoint, mName));
            
            return true;
        }
        
        
        void TransformationChainComponentInstance::apply(SpatialTransform& spatialTransform)
        {
            apply(spatialTransform.mPosition, spatialTransform.mScale, spatialTransform.mRotation);
        }
        
        
        void TransformationChainComponentInstance::apply(glm::vec3& position, glm::vec3& scale, glm::vec4& rotation)
        {
            for(auto& transformation : mTransformations)
                transformation->apply(position, scale, rotation);
        }

        
        void TransformationChainComponentInstance::applyExcluding(SpatialTransform& spatialTransform, std::string excludeFrom, std::string excludeUntil)
        {
            applyExcluding(spatialTransform.mPosition, spatialTransform.mScale, spatialTransform.mRotation, excludeFrom, excludeUntil);
        }
        
        
        void TransformationChainComponentInstance::applyExcluding(glm::vec3& position, glm::vec3& scale, glm::vec4& rotation, std::string excludeFrom, std::string excludeUntil)
        {
            bool apply = true;
            
            for(auto& transformation : mTransformations){
                
                if(transformation->getName() == excludeFrom)
                    apply = false;
                
                if(apply)
                    transformation->apply(position, scale, rotation);
                
                if(transformation->getName() == excludeUntil)
                    apply = true;
                
            }
        }

        
        void TransformationChainComponentInstance::applyExcluding(SpatialTransform& spatialTransform, std::string excludedTransformation)
        {
            applyExcluding(spatialTransform, excludedTransformation, excludedTransformation);
        }
        
        
        void TransformationChainComponentInstance::applyExcluding(glm::vec3& position, glm::vec3& scale, glm::vec4& rotation, std::string excludedTransformation)
        {
            applyExcluding(position, scale, rotation, excludedTransformation, excludedTransformation);
        }
        
        
        void TransformationChainComponentInstance::addExternalTransformation(std::string name, TransformationChainComponentInstance* transformationChain, bool defaultEnable, bool sharedEnable)
        {
            ExternalTransformation resource;
            resource.mName = name; // set name
            nap::utility::ErrorState errorState;
            std::unique_ptr<TransformationInstance> transformation = resource.instantiate(errorState, getEntityInstance(), *mAnchorPoint, mName);
            mTransformations.push_back(std::move(transformation));
            
            ExternalTransformationInstance* externalTransformation = rtti_cast<ExternalTransformationInstance>(mTransformations.back().get());
            externalTransformation->createEnableParameter(defaultEnable, sharedEnable);
            externalTransformation->setTransformationChain(transformationChain);
        }

        
        void TransformationChainComponentInstance::addSwitchExternalTransformation(std::string name, std::vector<TransformationChainComponentInstance*> transformationChains, std::vector<std::string> names)
        {
            SwitchExternalTransformation resource;
            resource.mName = name; // set name
            nap::utility::ErrorState errorState;
            std::unique_ptr<TransformationInstance> transformation = resource.instantiate(errorState, getEntityInstance(), *mAnchorPoint, mName);
            mTransformations.push_back(std::move(transformation));
            
            rtti_cast<SwitchExternalTransformationInstance>(mTransformations.back().get())->setTransformationChains(transformationChains, names);
        }
        
        
        void TransformationChainComponentInstance::update(double deltaTime)
        {
            for(auto& transformation : mTransformations)
                transformation->update(deltaTime);
        }
        
        
        TransformationInstance* TransformationChainComponentInstance::getTransformationByName(std::string name)
        {
            for(auto& transformation : mTransformations)
            {
                if(transformation->getName() == name)
                    return transformation.get();
            }
            return nullptr;
        }
        
        
    }
}
