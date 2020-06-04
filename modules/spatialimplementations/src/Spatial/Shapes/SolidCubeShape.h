#pragma once

#include <Spatial/Shape/Shape.h>

namespace nap
{
    namespace spatial
    {
        
        /**
         * Shape that completely covers the sound object transform by dividing it into rectangular cuboids of equal size.
         * It divides the sound object transform by creating an equal amounts of cuts in every axis.
         */
        class NAPAPI SolidCubeShapeInstance : public ShapeInstance
        {
            RTTI_ENABLE(ShapeInstance);
            
        public:
            SolidCubeShapeInstance(ShapeBase& shape) : ShapeInstance(shape) { }
            
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
        
        DECLARE_SHAPE(SolidCubeShape, SolidCubeShapeInstance)
        
    }
}
