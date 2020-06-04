#pragma once

#include <Spatial/Shape/Shape.h>
#include <Spatial/Core/SpatialFunctions.h>

namespace nap
{
    namespace spatial
    {
        
        /**
         * Shape that has a single particle with the same transform as the sound object itself.
         */
        class NAPAPI MonoShapeInstance : public ShapeInstance
        {
            RTTI_ENABLE(ShapeInstance);
            
        public:
            MonoShapeInstance(ShapeBase& shape) : ShapeInstance(shape) { }
            
            // Inherited from ShapeInstance
            virtual void onInit(nap::EntityInstance* entity) override;
            
            virtual std::vector<SpatialTransform> calculateParticleTransforms(const SpatialTransform& soundObjectTransform, double time) override;
            
            void getVerticesForVisualization(std::vector<glm::vec3>& outVertices) override;
            
            bool getEdgesForVisualization(std::vector<std::pair<int,int>> & outEdges) override;
            
        private:
            std::vector<glm::vec3> mVisualizationVertices;
            std::vector<std::pair<int,int>> mVisualizationEdges;
            
        };
        
        DECLARE_SHAPE(MonoShape, MonoShapeInstance)
        
    }
}
