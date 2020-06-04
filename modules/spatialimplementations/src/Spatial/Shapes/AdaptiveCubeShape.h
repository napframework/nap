#pragma once

#include <Spatial/Shape/Shape.h>

#include <stdio.h>

namespace nap
{
    namespace spatial
    {
        
        /**
         * Shape that completely covers the sound object transform by dividing it into rectangular cuboids of equal size.
         * It tries to keep the cuboids as most 'cube-like' as possible (which means, that when the sound object is stretched on an axis, instead of stretching the cuboids, it will spawn more cuboids on this axis in relation to the other axes)
         */

        class NAPAPI AdaptiveCubeShapeInstance : public ShapeInstance
        {
            RTTI_ENABLE(ShapeInstance);
            
        public:
            AdaptiveCubeShapeInstance(ShapeBase& shape) : ShapeInstance(shape) { }
            
            // Inherited from ShapeInstance
            virtual void onInit(nap::EntityInstance* entity) override;
            
            virtual std::vector<SpatialTransform> calculateParticleTransforms(const SpatialTransform& soundObjectTransform, double time) override;
            
            void getVerticesForVisualization(std::vector<glm::vec3>& outVertices) override
            {
                outVertices = mVisualizationVertices;
            }
            
            bool getEdgesForVisualization(std::vector<std::pair<int,int>> & outEdges) override
            {
                outEdges = mVisualizationEdges;
                return true;
            }
            
        private:
            std::vector<glm::vec3> mVisualizationVertices;
            std::vector<std::pair<int,int>> mVisualizationEdges;

            ParameterInt* mParticleCount = nullptr;
            
        };
        
        DECLARE_SHAPE(AdaptiveCubeShape, AdaptiveCubeShapeInstance)
        
    }
}
