
#include "SolidCubeShape.h"

#include <cmath>

// Spatial includes
#include <Spatial/Core/ParameterComponent.h>
#include <Spatial/Core/SpatialFunctions.h>


RTTI_DEFINE_CLASS(nap::spatial::SolidCubeShape)

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::spatial::SolidCubeShapeInstance)
    RTTI_CONSTRUCTOR(nap::spatial::ShapeBase&)
RTTI_END_CLASS

namespace nap
{
    namespace spatial
    {
        
        
        void SolidCubeShapeInstance::onInit(nap::EntityInstance* entity)
        {
            mVisualizationVertices = {
                glm::vec3(-.5, -.5, -.5),
                glm::vec3(-.5, -.5, .5),
                glm::vec3(-.5, .5, -.5),
                glm::vec3(-.5, .5, .5),
                glm::vec3(.5, -.5, -.5),
                glm::vec3(.5, -.5, .5),
                glm::vec3(.5, .5, -.5),
                glm::vec3(.5, .5, .5)
            };
            
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
                std::pair<int,int>(6, 4)
            };
            
            mParticleCount = getParameterManager().addParameterInt("particleCount", 1, 1, 1000, true);
            
        }
        
        std::vector<SpatialTransform> SolidCubeShapeInstance::calculateParticleTransforms(const SpatialTransform& soundObjectTransform, double time)
        {
    
            std::vector<SpatialTransform> particles;
            
            int density = std::cbrt(mParticleCount->mValue);

            glm::vec3 scale = soundObjectTransform.mScale * (1.f / density);
            
            for(int x = 0; x < density; x++)
            {
                for(int y = 0; y < density; y++)
                {
                    for(int z = 0; z < density; z++)
                    {
                        float xPosition = (((x * 2.f + 1.f) / (2.f * density))) * 2.f - 1.f;
                        float yPosition = (((y * 2.f + 1.f) / (2.f * density))) * 2.f - 1.f;
                        float zPosition = (((z * 2.f + 1.f) / (2.f * density))) * 2.f - 1.f;
                        glm::vec3 position(xPosition, yPosition, zPosition);
                        
                        particles.emplace_back(GlmFunctions::objectSpaceToWorldSpace(position, soundObjectTransform), scale, soundObjectTransform.mRotation);
                    }
                }
            }
            
            return particles;
        }

    }
}
