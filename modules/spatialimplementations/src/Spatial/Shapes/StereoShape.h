#pragma once

#include <Spatial/Shape/Shape.h>

namespace nap
{
    namespace spatial
    {
        
        /**
         * Simple shape that consists of two particles with the size of the sound object transform.
         * The 'separation' parameter determines the space between the two particles.
         */
        class NAPAPI StereoShapeInstance : public ShapeInstance
        {
            RTTI_ENABLE(ShapeInstance);
            
        public:
            StereoShapeInstance(ShapeBase& shape) : ShapeInstance(shape) { }
            
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
            
            void updateVisualization();
            
            std::vector<glm::vec3> mCubeVertices;
            
            std::vector<glm::vec3> mVisualizationVertices;
            std::vector<std::pair<int,int>> mVisualizationEdges;

            ParameterFloat* mSeparation = nullptr;
            
        };

        DECLARE_SHAPE(StereoShape, StereoShapeInstance)
        
    }
}
