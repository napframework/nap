
#include "PythonShape.h"

#include <Spatial/Core/SpatialFunctions.h>

RTTI_BEGIN_CLASS(nap::spatial::PythonShape)
    RTTI_PROPERTY("PythonScript", &nap::spatial::PythonShape::mPythonScript, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("Class", &nap::spatial::PythonShape::mClassName, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::spatial::PythonShapeInstance)
RTTI_END_CLASS


namespace nap
{
    namespace spatial
    {
    
        bool PythonShape::init(utility::ErrorState& errorState)
        {
            mPythonClass = mPythonScript->get(mClassName);
            if (mPythonClass.is_none())
                return false;
            
            return true;
        }
        
        
        void PythonShapeInstance::onInit(nap::EntityInstance* entity)
        {
            auto* resource = getShape<PythonShape>();
            mPythonClassInstance.init(resource->mPythonScript.get(), &resource->mPythonClass, entity, getName(), &getParameterManager());
            
            // same particle count as the works shapes
            mParticleCount = getParameterManager().addParameterInt("particleCount", 1, 1, 1000, true);
            
            mEntity = entity;
        }
        
        
        void PythonShapeInstance::update(double deltaTime)
        {
            mPythonClassInstance.call<void>("update", deltaTime);
        }
        
        
        std::vector<SpatialTransform> PythonShapeInstance::calculateParticleTransforms(const SpatialTransform& soundObjectTransform, double time)
        {
            std::vector<SpatialTransform> transforms = mPythonClassInstance.call<std::vector<SpatialTransform>>("getParticles", mParticleCount ? mParticleCount->mValue : 0, time);
            
            // convert to world space
            for(int i = 0; i < transforms.size(); i++)
                transforms[i] = SpatialTransform(GlmFunctions::shapeSpaceToWorldSpace(transforms[i].mPosition * soundObjectTransform.mScale, soundObjectTransform), transforms[i].mScale * soundObjectTransform.mScale, GlmFunctions::compositeAngleAxisRotations(transforms[i].mRotation, soundObjectTransform.mRotation));
            
            return transforms;
        }
        
        
        void PythonShapeInstance::getVerticesForVisualization(std::vector<glm::vec3>& outVertices)
        {
            outVertices = mPythonClassInstance.call<std::vector<glm::vec3>>("getVisualizationVertices");
        }
        
        
        bool PythonShapeInstance::getEdgesForVisualization(std::vector<std::pair<int,int>> & outEdges)
        {
            outEdges = mPythonClassInstance.call<std::vector<std::pair<int,int>>>("getVisualizationEdges");
            return outEdges.size() > 0;
        }
    
    }
}
