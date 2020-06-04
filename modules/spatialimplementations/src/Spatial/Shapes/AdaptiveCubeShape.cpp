#include "AdaptiveCubeShape.h"

#include <cmath>

// Spatial includes
#include <Spatial/Core/ParameterComponent.h>
#include <Spatial/Core/SpatialFunctions.h>


RTTI_DEFINE_CLASS(nap::spatial::AdaptiveCubeShape)

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::spatial::AdaptiveCubeShapeInstance)
    RTTI_CONSTRUCTOR(nap::spatial::ShapeBase&)
RTTI_END_CLASS

namespace nap
{
    namespace spatial
    {
        
        
        void AdaptiveCubeShapeInstance::onInit(nap::EntityInstance* entity)
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
        
        std::vector<SpatialTransform> AdaptiveCubeShapeInstance::calculateParticleTransforms(const SpatialTransform& soundObjectTransform, double time)
        {
            int particleCount = mParticleCount->mValue;
            
            // find aspect ratio per axis
            float yMultiplier = soundObjectTransform.mScale.y / soundObjectTransform.mScale.x;
            float zMultiplier = soundObjectTransform.mScale.z / soundObjectTransform.mScale.x;
            
            // Now, to find xDensity, solve the equation: x * yMultiplier * x * zMultiplier * x = particlecount
            // >> yMultiplier * zMultiplier * x^3 = particlecount
            // >> x^3 = particlecount / (yM * zM)
            // >> x = sqrt3(particlecount / (yM * zM))
            float xDensityAsFloat = std::cbrt(particleCount / (yMultiplier * zMultiplier));
            
            // Now xDensity has been found, yDensity and zDensity can be calculated by multiplying with the aspect ratios:
            int xDensity = std::max<int>(1, round(xDensityAsFloat));
            int yDensity = std::max<int>(1, round(xDensityAsFloat * yMultiplier));
            int zDensity = std::max<int>(1, round(xDensityAsFloat * zMultiplier));
            
            std::vector<SpatialTransform> particles;
            
            glm::vec3 scale(soundObjectTransform.mScale.x / xDensity, soundObjectTransform.mScale.y / yDensity, soundObjectTransform.mScale.z / zDensity);
            
            for(int x = 0; x < xDensity; x++)
            {
                for(int y = 0; y < yDensity; y++)
                {
                    for(int z = 0; z < zDensity; z++)
                    {
                        float xPosition = (((x * 2.f + 1.f) / (2.f * xDensity))) * 2.f - 1.f;
                        float yPosition = (((y * 2.f + 1.f) / (2.f * yDensity))) * 2.f - 1.f;
                        float zPosition = (((z * 2.f + 1.f) / (2.f * zDensity))) * 2.f - 1.f;
                        glm::vec3 position(xPosition, yPosition, zPosition);
                        
                        particles.emplace_back(GlmFunctions::objectSpaceToWorldSpace(position, soundObjectTransform), scale, soundObjectTransform.mRotation);
                    }
                }
            }
            
            return particles;
        }
        
    }
}
