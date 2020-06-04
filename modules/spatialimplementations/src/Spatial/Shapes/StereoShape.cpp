
#include "StereoShape.h"

// Spatial includes
#include <Spatial/Core/ParameterComponent.h>
#include <Spatial/Core/SpatialFunctions.h>


RTTI_DEFINE_CLASS(nap::spatial::StereoShape)

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::spatial::StereoShapeInstance)
    RTTI_CONSTRUCTOR(nap::spatial::ShapeBase&)
RTTI_END_CLASS

namespace nap
{
    namespace spatial
    {
        
        void StereoShapeInstance::onInit(nap::EntityInstance* entity)
        {
            mSeparation = getParameterManager().addParameterFloat("separation", 1.0, 0.0, 10.0);
            mSeparation->valueChanged.connect([&](float){ updateVisualization(); });
            
            mVisualizationVertices = {};
            
            mCubeVertices = {
                glm::vec3(-.5, -.5, -.5),
                glm::vec3(-.5, -.5, .5),
                glm::vec3(-.5, .5, -.5),
                glm::vec3(-.5, .5, .5),
                glm::vec3(.5, -.5, -.5),
                glm::vec3(.5, -.5, .5),
                glm::vec3(.5, .5, -.5),
                glm::vec3(.5, .5, .5)
            };

            
            // 2 cubes.
            mVisualizationEdges = {
                std::pair<int,int>(0, 1),
                std::pair<int,int>(1, 3),
                std::pair<int,int>(3, 2),
                std::pair<int,int>(2, 0),
                std::pair<int,int>(0, 4),
                std::pair<int,int>(1, 5),
                std::pair<int,int>(2, 6),
                std::pair<int,int>(3, 7),
                std::pair<int,int>(4, 5),
                std::pair<int,int>(5, 7),
                std::pair<int,int>(7, 6),
                std::pair<int,int>(6, 4),
                
                std::pair<int,int>(8, 9),
                std::pair<int,int>(9, 11),
                std::pair<int,int>(11, 10),
                std::pair<int,int>(10, 8),
                std::pair<int,int>(8, 12),
                std::pair<int,int>(9, 13),
                std::pair<int,int>(10, 14),
                std::pair<int,int>(11, 15),
                std::pair<int,int>(12, 13),
                std::pair<int,int>(13, 15),
                std::pair<int,int>(15, 14),
                std::pair<int,int>(14, 12)

            };
            
            updateVisualization();
            
        }
        
        
        std::vector<SpatialTransform> StereoShapeInstance::calculateParticleTransforms(const SpatialTransform& soundObjectTransform, double time)
        {
            
            SpatialTransform left(GlmFunctions::shapeSpaceToWorldSpace(glm::vec3(-0.5 * mSeparation->mValue * soundObjectTransform.mScale.x, 0., 0.), soundObjectTransform),
                                  soundObjectTransform.mScale,
                                  soundObjectTransform.mRotation);
            
            SpatialTransform right(GlmFunctions::shapeSpaceToWorldSpace(glm::vec3(0.5 * mSeparation->mValue * soundObjectTransform.mScale.x, 0., 0.), soundObjectTransform),
                                   soundObjectTransform.mScale,
                                   soundObjectTransform.mRotation);
            
            return { left, right };
        }
        
        
        void StereoShapeInstance::updateVisualization()
        {
            mVisualizationVertices.clear();
            
            glm::vec3 offset(0.5f * mSeparation->mValue, 0., 0.);
            
            for(auto& cubeVertex : mCubeVertices)
                mVisualizationVertices.push_back(cubeVertex - offset);
            for(auto& cubeVertex : mCubeVertices)
                mVisualizationVertices.push_back(cubeVertex + offset);
            
        }
        
    }
}
