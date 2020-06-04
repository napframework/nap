#pragma once

#include <Spatial/Shape/Shape.h>

namespace nap
{
    namespace spatial
    {
        
        /**
         * A shape that fills the walls of the sound object transform with two-dimensional tiles, ordered in a chessboard pattern. 
         * The number of tiles is based on the requested particle count.
         * left, right, top, bottom, front, back (bool): individual toggles for all 6 faces.
         * distance: multiplies the distance of every tile to the center of the sound object (like plode)
         */
        class NAPAPI WallsShapeInstance : public ShapeInstance
        {
            RTTI_ENABLE(ShapeInstance);
            
        public:
            WallsShapeInstance(ShapeBase& shape) : ShapeInstance(shape) { }
            
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

            std::vector<glm::vec3> mVisualizationVertices;
            std::vector<std::pair<int,int>> mVisualizationEdges;
            int mCurrentDensity = 1;

            ParameterBool* mLeft = nullptr;
            ParameterBool* mRight = nullptr;
            ParameterBool* mTop = nullptr;
            ParameterBool* mBottom = nullptr;
            ParameterBool* mFront = nullptr;
            ParameterBool* mBack = nullptr;
            
            ParameterInt* mParticleCount = nullptr;
            
            ParameterFloat* mDistance = nullptr;            

        };
        
        DECLARE_SHAPE(WallsShape, WallsShapeInstance)
        
    }
}
