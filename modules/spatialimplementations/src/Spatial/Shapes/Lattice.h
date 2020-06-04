#pragma once

#include <vector>
#include <glm/vec3.hpp>
#include <utility/dllexport.h>

namespace nap
{
    
    namespace spatial
    {

        // math helper functions returning all positions of a certain fillType within a bounding square (or cube)
        std::vector<glm::vec2> getSquareLatticePositionsWithinBoundingSquare(const glm::vec2& boundingSquare, const float& latticeSize);
        std::vector<glm::vec2> getTriangleLatticePositionsWithinBoundingSquare(const glm::vec2& boundingSquare, const float& latticeSize);
        std::vector<glm::vec3> getCubeLatticePositionsWithinBoundingCube(const glm::vec3& boundingCube, const float& latticeSize);

        std::vector<glm::vec3> getTriangularPrismaticLatticePositionsWithinBoundingCube(const glm::vec3& boundingCube, const float& latticeSize);
        std::vector<glm::vec3> getTetrahedronOctahedronLatticePositionsWithinBoundingCube(const glm::vec3& boundingCube, const float& latticeSize);
        std::vector<glm::vec3> getShiftedCubeLatticePositionsWithinBoundingCube(const glm::vec3& boundingCube, const float& latticeSize);

        /**
         * Baseclass.
         * A Lattice calculates positions of an even lattice with type 'FillType' and size 'latticeSize' within a shape scaled by 'dimensions'.
         **/
        class NAPAPI Lattice {
            
        public:
            enum FillType { Cubic, Triangular, ShiftedCubic, TetrahedronOctahedron };
            
            Lattice() = default;
			
            virtual ~Lattice() = default;
            
            virtual std::vector<glm::vec3> calculatePositions(glm::vec3 dimensions, float latticeSize, FillType fillType) =0;
            
            /**
             * 2D helper function to get the particle positions within a bounding square according to the fillType.
             */
            std::vector<glm::vec2> getLatticePositionsWithinBoundingSquare(const glm::vec2& boundingSquare, const float& latticeSize, const Lattice::FillType& fillType);
            
            /**
             * 3D helper function to get the particle positions within a bounding cube according to the fillType.
             */
            std::vector<glm::vec3> getLatticePositionsWithinBoundingCube(const glm::vec3& boundingCube, const float& latticeSize, const Lattice::FillType& fillType);
            
        };


        
        

        /**
         * Line Lattice.
         */
        class LineLattice : public Lattice {
            
        public:
            LineLattice() = default;
            
            virtual std::vector<glm::vec3> calculatePositions(glm::vec3 dimensions, float latticeSize, FillType fillType) override;
        };


        /**
         * Circle Lattice.
         */
        class CircleLattice : public Lattice {
            
        public:
            CircleLattice() = default;
            
            virtual std::vector<glm::vec3> calculatePositions(glm::vec3 dimensions, float latticeSize, FillType fillType) override;
        };

        
        /**
         * Lattice inside a two-dimensions polygon.
         */
        class Polygon2DLattice : public Lattice {
            
        public:
            Polygon2DLattice(std::vector<glm::vec2> polygon) : Lattice(), polygon(polygon) {}
            
            virtual std::vector<glm::vec3> calculatePositions(glm::vec3 dimensions, float latticeSize, FillType fillType) override;
            
        private:
            std::vector<glm::vec2> polygon;
            
        };
        
        
        /**
         * Sphere Lattice.
         */
        class SphereLattice : public Lattice {
        
        public:
            SphereLattice() = default;
            
            virtual std::vector<glm::vec3> calculatePositions(glm::vec3 dimensions, float latticeSize, FillType fillType) override;
        };
        
        /**
         * Cube Lattice.
         */
        class CubeLattice : public Lattice {
           
        public:
            CubeLattice() = default;
            
            virtual std::vector<glm::vec3> calculatePositions(glm::vec3 dimensions, float latticeSize, FillType fillType) override {
                return getLatticePositionsWithinBoundingCube(dimensions, latticeSize, fillType);
            }
        };
        
        /**
         * Lattice inside a three-dimensional polygon.
         */
        class Polygon3DLattice : public Lattice {
            
        public:
            Polygon3DLattice(std::vector<glm::vec3> vertices, std::vector<std::vector<int>> faces);
            virtual std::vector<glm::vec3> calculatePositions(glm::vec3 dimensions, float latticeSize, FillType fillType) override;
            
        private:
            std::vector<glm::vec3> faceNormals;
            std::vector<glm::vec3> faceOrigins;
        };
        
        
        
        
        
        // TODO TODO alleen voor nu even hier. Verplaatsen naar TheWorksShape.cpp oid.
        // Math helper functions.
        
        /*
         * Populates a shape defined by inputVertices by evenly dividing it into 'res' amount concentric shapes.
         * 'edgePointCount' is the amount of particles on an edge until a new concentric shape will spawn inside.
         * (e.g. for triangles this is 3, squares 2, pentagon/hexagon 1)
         * If 'inner' is true, the inner concentric shapes will be populated
         * if 'outer' is true, the outer concentric shape will be populated
         * 'edges' is the list of edges between vertices (by indexes).
         */
        
        std::vector<glm::vec3> populateShape(
                                             const std::vector<glm::vec3>& inputVertices,
                                             const std::vector<std::pair<int,int>>& edges,
                                             const int res,
                                             const int edgePointCount,
                                             const bool inner, const bool outer,
                                             const bool hasCenterPoint = false,
                                             const glm::vec3 centerPoint = glm::vec3(0.,0.,0.));
        
        std::vector<std::pair<int,int>> getStraightEdges(int count);
        
        const std::vector<glm::vec3>& getVectorWithClosestSize(const std::vector<std::vector<glm::vec3>>& vec, int count);
        
        void scalePositions(glm::vec3 scale, std::vector<glm::vec3>& positions);
        // _______________________________________________________________________

    
    }
}
