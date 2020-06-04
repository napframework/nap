
#include "MonoShape.h"

RTTI_DEFINE_CLASS(nap::spatial::MonoShape)

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::spatial::MonoShapeInstance)
    RTTI_CONSTRUCTOR(nap::spatial::ShapeBase&)
RTTI_END_CLASS


namespace nap
{
    namespace spatial
    {
        
        void MonoShapeInstance::onInit(nap::EntityInstance* entity) {
            
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
            
        }
        
        
        std::vector<SpatialTransform> MonoShapeInstance::calculateParticleTransforms(const SpatialTransform& soundObjectTransform, double time)
        {
            return { soundObjectTransform };
        }
        
        void MonoShapeInstance::getVerticesForVisualization(std::vector<glm::vec3>& outVertices)
        {
            outVertices = mVisualizationVertices;
        }
        
        bool MonoShapeInstance::getEdgesForVisualization(std::vector<std::pair<int,int>> & outEdges)
        {
            outEdges = mVisualizationEdges;
            return true;
        }
        
    }
}
