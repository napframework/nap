#include "ShapeComponent.h"

// Spatial includes
#include <Spatial/Core/ParameterComponent.h>
#include <Spatial/Core/SpatialTypes.h>
#include <Spatial/Utility/ParameterTypes.h>
#include <Spatial/Shape/Shape.h>
#include <Spatial/Shape/Deform.h>

// Nap includes
#include <entity.h>


// RTTI
RTTI_BEGIN_CLASS(nap::spatial::ShapeComponent)
    RTTI_PROPERTY("Shapes", &nap::spatial::ShapeComponent::mShapes, nap::rtti::EPropertyMetaData::Required | nap::rtti::EPropertyMetaData::Embedded);
    RTTI_PROPERTY("Deforms", &nap::spatial::ShapeComponent::mDeforms, nap::rtti::EPropertyMetaData::Embedded);
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::spatial::ShapeComponentInstance)
    RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

namespace nap
{
    namespace spatial
    {
        
        void ShapeComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
        {
            components.push_back(RTTI_OF(ParameterComponent));
        }
        
        ShapeComponentInstance::ShapeComponentInstance(EntityInstance& entity, Component& resource) : ComponentInstance(entity, resource)
        {
        }        
        
        bool ShapeComponentInstance::init(utility::ErrorState& errorState)
        {
            
            // Instantiate shape instances.
            for(auto& shapeResource : getComponent<ShapeComponent>()->mShapes)
            {
                mShapes.push_back(shapeResource->instantiate(errorState, getEntityInstance()));
            }
            
            // Fail if no shapes are loaded.
            if(errorState.check(!mShapes.empty(), "No shapes in ShapeComponent."))
                mCurrentShape = mShapes[0].get();
            else
                return false;
            
            // Instantiate deform instances.
            for(auto& deformResource : getComponent<ShapeComponent>()->mDeforms)
            {
                mDeforms.push_back(deformResource->instantiate(errorState, getEntityInstance()));
            }
            
            
            // Add parameters.
            ParameterComponentInstance* parameterComponent = getEntityInstance()->findComponent<ParameterComponentInstance>();
            
            std::vector<std::string> typeNames;
            for(auto& shape : mShapes){
                if(!errorState.check(std::find(typeNames.begin(), typeNames.end(), shape->getName()) == typeNames.end(), "Duplicate shape."))
                    return false;
                
                typeNames.push_back(shape->getName());
            }
            
            mShapeType = &parameterComponent->addParameter<ParameterOptionList>("shapeType", 0);
            mShapeType->setOptions(typeNames);
            mShapeType->connect([&](int typeIndex){
                mCurrentShape = mShapes[typeIndex].get();
            } );
            
            mSpeed = &parameterComponent->addParameterFloat("speed", 1.0, -10.0, 10.0);
            
            return true;
        }
        
        
        std::vector<SpatialTransform> ShapeComponentInstance::calculateParticleTransforms(const SpatialTransform& soundObjectTransform)
        {
            
            auto positions = mCurrentShape->calculateParticleTransforms(soundObjectTransform, mTime);
            
            for(auto& deform : mDeforms)
                deform->apply(positions, soundObjectTransform);
            
            return positions;

        }


        void ShapeComponentInstance::update(double deltaTime)
        {
            mTime += mSpeed->mValue * deltaTime;

            for(auto& shape : mShapes)
                shape->update(deltaTime);

            for(auto& deform : mDeforms)
                deform->update(deltaTime);
        }


        void ShapeComponentInstance::getVerticesForVisualization(std::vector<glm::vec3>& out_vertices)
        {
            mCurrentShape->getVerticesForVisualization(out_vertices);
        }

        
        bool ShapeComponentInstance::getEdgesForVisualization(std::vector<std::pair<int,int>> & out_edges)
        {
            return mCurrentShape->getEdgesForVisualization(out_edges);
        }

    }
}
