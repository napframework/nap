
#include "Lattice.h"

// Spatial includes
// General math functions like pointInPoly are now inside casipan2 folder, should maybe be on a more central place?
#include <Spatial/MultiSpeaker/casipan2/Functions.hpp>

// GLM includes
#include "glm/gtx/hash.hpp"
#include <glm/ext.hpp>

// Std includes
#include <memory>
#include <unordered_set>


namespace nap
{
    namespace spatial
    {
        
        
        // Class methods.
        
        std::vector<glm::vec2> Lattice::getLatticePositionsWithinBoundingSquare(const glm::vec2& boundingSquare, const float& latticeSize, const Lattice::FillType& fillType)
        {
            
            if(fillType == Lattice::Triangular)
                return getTriangleLatticePositionsWithinBoundingSquare(boundingSquare, latticeSize);
            else // square = default
                return getSquareLatticePositionsWithinBoundingSquare(boundingSquare, latticeSize);
            
        }
        
        std::vector<glm::vec3> Lattice::getLatticePositionsWithinBoundingCube(const glm::vec3& boundingCube, const float& latticeSize, const Lattice::FillType& fillType)
        {
            
            if(fillType == Triangular)
                return getTriangularPrismaticLatticePositionsWithinBoundingCube(boundingCube, latticeSize);
            else if(fillType == ShiftedCubic)
                return getShiftedCubeLatticePositionsWithinBoundingCube(boundingCube, latticeSize);
            else if(fillType == TetrahedronOctahedron)
                return getTetrahedronOctahedronLatticePositionsWithinBoundingCube(boundingCube, latticeSize);
            else // cubic
                return getCubeLatticePositionsWithinBoundingCube(boundingCube, latticeSize);
        }

        
        
        std::vector<glm::vec3> LineLattice::calculatePositions(glm::vec3 dimensions, float latticeSize, FillType fillType)
        {
            
            std::vector<glm::vec3> positions;
            positions.push_back(glm::vec3(0.,0.,0.));
            
            for(float x = latticeSize; x < .5 * dimensions.x; x += latticeSize){
                positions.push_back(glm::vec3(x, 0., 0.));
                positions.push_back(glm::vec3(-x, 0., 0.));
            }
            
            return positions;
            
        }
        
        
        std::vector<glm::vec3> CircleLattice::calculatePositions(glm::vec3 dimensions, float latticeSize, FillType fillType)
        {
            
            std::vector<glm::vec3> positions;
            
            glm::vec2 boundingSquare(dimensions.x, dimensions.z);
            
            std::vector<glm::vec2> positionsWithinBoundingSquare = getLatticePositionsWithinBoundingSquare(boundingSquare, latticeSize, fillType);
            
            for(auto& position : positionsWithinBoundingSquare){
                // scale back position
                glm::vec2 scaledBackPosition = position / boundingSquare;
                
                // check if the scaledBackPositions falls within a circle of width 1 around the origin. If so, add the position.
                if(glm::length(scaledBackPosition) < .5)
                    positions.emplace_back(position.x, 0, position.y);
            }
            
            return positions;
            
        }

        
        std::vector<glm::vec3> Polygon2DLattice::calculatePositions(glm::vec3 dimensions, float latticeSize, FillType fillType)
        {
            
            std::vector<glm::vec3> positions;
            glm::vec2 boundingSquare(dimensions.x, dimensions.z);
            
            std::vector<glm::vec2> positionsWithinBoundingSquare = getLatticePositionsWithinBoundingSquare(boundingSquare, latticeSize, fillType);
            
            for(auto& position : positionsWithinBoundingSquare){
                if(casipan::pointInPoly(position / boundingSquare, polygon))
                    positions.emplace_back(position.x, 0, position.y);
            }
            
            return positions;
        }
        
        std::vector<glm::vec3> SphereLattice::calculatePositions(glm::vec3 dimensions, float latticeSize, FillType fillType)
        {
            
            std::vector<glm::vec3> positions;
            std::vector<glm::vec3> positionsWithinBoundingCube = getLatticePositionsWithinBoundingCube(dimensions, latticeSize, fillType);
            
            for(auto& position : positionsWithinBoundingCube){
                if(glm::length(position / dimensions) < .5)
                    positions.emplace_back(position);
            }
            
            return positions;
        }


        
        Polygon3DLattice::Polygon3DLattice(std::vector<glm::vec3> vertices, std::vector<std::vector<int>> faces) : Lattice()
        {
            
            // calculate outward facing normals from vertices & faces.
            for(auto& face : faces){
                
                // calculate normal
                glm::vec3 normal = glm::normalize(glm::cross(vertices[face[1]] - vertices[face[0]], vertices[face[2]] - vertices[face[0]]));
                // if inward facing, flip sign.
                if(glm::dot(normal, vertices[face[0]]) < 0)
                    normal = -normal;
                
                // push back normal and a point on the face.
                faceNormals.push_back(normal);
                faceOrigins.push_back(vertices[face[0]]);
            }
            
        }
        
        std::vector<glm::vec3> Polygon3DLattice::calculatePositions(glm::vec3 dimensions, float latticeSize, FillType fillType)
        {
            
            std::vector<glm::vec3> positions;
            
            std::vector<glm::vec3> positionsWithinBoundingCube = getLatticePositionsWithinBoundingCube(dimensions, latticeSize, fillType);
            
            for(auto& position : positionsWithinBoundingCube){
                
                bool outside = false;
                
                for(int i = 0; i < faceNormals.size(); i++){
                    if(glm::dot((position / dimensions) - faceOrigins[i], faceNormals[i]) > 0.){
                        outside = true;
                        continue;
                    }
                }
                
                if(!outside)
                    positions.emplace_back(position);
                
            }
            
            return positions;
            
        }

        
        
        // Lattice Filling Functions.
        
        // 2D.
        
        std::vector<glm::vec2> getSquareLatticePositionsWithinBoundingSquare(const glm::vec2& boundingSquare, const float& latticeSize)
        {
            std::vector<glm::vec2> positions;
            
            positions.push_back(glm::vec2(0,0));
            
            for(float y = latticeSize; y < .5 * boundingSquare.y; y+= latticeSize){
                positions.push_back(glm::vec2(0,y));
                positions.push_back(glm::vec2(0,-y));
            }
            
            for(float x = latticeSize; x < .5 * boundingSquare.x; x+= latticeSize){
                positions.push_back(glm::vec2(x, 0.));
                positions.push_back(glm::vec2(-x, 0.));
                for(float y = latticeSize; y < .5 * boundingSquare.y; y+= latticeSize){
                    positions.push_back(glm::vec2(x,y));
                    positions.push_back(glm::vec2(x,-y));
                    positions.push_back(glm::vec2(-x,y));
                    positions.push_back(glm::vec2(-x,-y));
                }
            }
            
            return positions;
        }
        
        std::vector<glm::vec2> getTriangleLatticePositionsWithinBoundingSquare(const glm::vec2& boundingSquare, const float& latticeSize)
        {
            std::vector<glm::vec2> positions;
            
            glm::vec2 v2(0.5, 0.8660254); // 1/2, 1/2sqrt(3)
            
            float yIncrement = 1.7320508076 * latticeSize; // sqrt(3)
            
            glm::vec2 offset = v2 * latticeSize;
            
            positions.push_back(glm::vec2(0,0));
            positions.push_back(offset);
            
            for(float y = yIncrement; y < .5 * boundingSquare.y + offset.y; y+= yIncrement){
                positions.push_back(glm::vec2(0,y));
                positions.push_back(glm::vec2(0,-y));
                positions.push_back(glm::vec2(0,y) + offset);
                positions.push_back(glm::vec2(0,-y) + offset);
                
            }
            
            for(float x = latticeSize; x < .5 * boundingSquare.x + offset.x; x+= latticeSize){
                positions.push_back(glm::vec2(x, 0.));
                positions.push_back(glm::vec2(-x, 0.));
                positions.push_back(glm::vec2(x, 0.) + offset);
                positions.push_back(glm::vec2(-x, 0.) + offset);
                for(float y = yIncrement; y < .5 * boundingSquare.y + offset.y; y+= yIncrement ){
                    positions.push_back(glm::vec2(x,y));
                    positions.push_back(glm::vec2(x,-y));
                    positions.push_back(glm::vec2(-x,y));
                    positions.push_back(glm::vec2(-x,-y));
                    positions.push_back(glm::vec2(x,y) + offset);
                    positions.push_back(glm::vec2(x,-y) + offset);
                    positions.push_back(glm::vec2(-x,y) + offset);
                    positions.push_back(glm::vec2(-x,-y) + offset);
                }
            }
            
            return positions;
            
        }

        
        // 3D helper functions.
        
        
        struct TreeNode {
            
        public:
            TreeNode(TreeNode* parent, glm::ivec3 latticePosition, glm::vec3 realPosition) : parent(parent), latticePosition(latticePosition), realPosition(realPosition)
            {
                children.reserve(8); // to prevent pointer invalidation
            }
            TreeNode* parent;
            glm::ivec3 latticePosition;
            glm::vec3 realPosition;
            std::vector<TreeNode*> children;
            bool checked = false;
        };
        
        float isInsideBoundingBox(const glm::vec3& pos, const glm::vec3& box)
        {
            return  pos.x > -.5 * box.x && pos.x < .5 * box.x &&
                    pos.y > -.5 * box.y && pos.y < .5 * box.y &&
                    pos.z > -.5 * box.z && pos.z < .5 * box.z;
        }
        
        // fills bounding box with lattice with a tree expansion/flooding algorithm, starting from the origin.
        std::vector<glm::vec3> fillBoundingCubeWithLattice(const glm::vec3& latticeVector1, const glm::vec3& latticeVector2, const glm::vec3& latticeVector3, const glm::vec3& boundingBox, const float& latticeSize)
        {
            
            std::vector<glm::vec3> positions;
            
            // a set of lattice positions to keep track which positions have already been added to the tree.
            std::unordered_set<glm::ivec3> addedLatticePositions;
            
            std::vector<std::unique_ptr<TreeNode>> tree;

            TreeNode* currentNode;
            
            tree.push_back(std::make_unique<TreeNode>(nullptr, glm::ivec3(0,0,0), glm::vec3(0,0,0)));
            addedLatticePositions.insert(glm::ivec3(0,0,0));
            currentNode = tree.back().get();
            
            while(true)
            {
                
                if(!currentNode->checked)
                {
                    
                    // currentNode will be checked now.
                    currentNode->checked = true;
                    
                    // check if the real position of the current node is inside the bounding box.
                    if(isInsideBoundingBox(currentNode->realPosition, boundingBox))
                    {
                        
                        // if so, add it to the output positions.
                        positions.push_back(currentNode->realPosition);
                        
                        // and spawn children in all directions, if they haven't been added before.
                        std::vector<glm::ivec3> newLatticePositions = {
                            (currentNode->latticePosition + glm::ivec3(-1, 0, 0)),
                            (currentNode->latticePosition + glm::ivec3(1, 0, 0)),
                            (currentNode->latticePosition + glm::ivec3(0, -1, 0)),
                            (currentNode->latticePosition + glm::ivec3(0, 1, 0)),
                            (currentNode->latticePosition + glm::ivec3(0, 0, -1)),
                            (currentNode->latticePosition + glm::ivec3(0, 0, 1)) };
                        
                        for(auto& newLatticePosition : newLatticePositions)
                        {
                            
                            // add if the position hasn't been added before
                            if(std::find(addedLatticePositions.begin(), addedLatticePositions.end(), newLatticePosition) == addedLatticePositions.end()){
                                
                                // calculate real position
                                glm::vec3 realPosition = newLatticePosition.x * latticeSize * latticeVector1 + newLatticePosition.y * latticeSize * latticeVector2 + newLatticePosition.z * latticeSize * latticeVector3;
                                
                                tree.push_back(std::make_unique<TreeNode>(currentNode, newLatticePosition, realPosition));
                                currentNode->children.push_back(tree.back().get());
                                addedLatticePositions.insert(newLatticePosition);
                            }
                        }
                        
                        // select the first child.
                        if(currentNode->children.size() > 0)
                        {
                            currentNode = currentNode->children[0];
                        }
                        
                        
                    }
                    else{ // the real position doesn't fall inside the bounding box..
                        
                        // go back to parent
                        currentNode = currentNode->parent;
                        
                    }
                
                }
                else{ // current node has been checked already.
                    
                    // select first child that hasn't been checked already.
                    // If all children are checked, go back to parent.
                    bool goBack = true;
                    
                    if(currentNode->children.size() > 0)
                    {
                        for(auto child : currentNode->children)
                        {
                            if(!child->checked){
                                currentNode = child;
                                goBack = false;
                                break;
                            }
                            
                        }
                    }
                    
                    if(goBack)
                        currentNode = currentNode->parent;

                }
                
                // break if we are completely back at the start (the parent of the starting node is nullptr).
                if(currentNode == nullptr)
                    break;

                
            }
            
            return positions;
            
        }

        
        // 3D.
        
        std::vector<glm::vec3> getCubeLatticePositionsWithinBoundingCube(const glm::vec3& boundingCube, const float& latticeSize)
        {
            
            std::vector<glm::vec3> positions;
            
            std::vector<glm::vec2> squarePositions = getSquareLatticePositionsWithinBoundingSquare(casipan::to2D(boundingCube),latticeSize);
            for(auto& pos : squarePositions)
            {
                positions.emplace_back(pos.x, 0, pos.y);
            }

            for(float height = latticeSize; height < .5 * boundingCube.y; height += latticeSize)
            {
                std::vector<glm::vec2> squarePositions = getSquareLatticePositionsWithinBoundingSquare(casipan::to2D(boundingCube),latticeSize);
                for(auto& pos : squarePositions)
                {
                    positions.emplace_back(pos.x, height, pos.y);
                    positions.emplace_back(pos.x, -height, pos.y);
                }
            }
            
            return positions;
        }
        
        
        std::vector<glm::vec3> getTriangularPrismaticLatticePositionsWithinBoundingCube(const glm::vec3& boundingCube, const float& latticeSize)
        {
            
            glm::vec3 v1(1., 0., 0.);
            glm::vec3 v2(0.5, 0.5 * sqrt(3.), 0.);
            glm::vec3 v3(0., 0., 1.);
            
            return fillBoundingCubeWithLattice(v1, v2, v3, boundingCube, latticeSize);
        }
        
        std::vector<glm::vec3> getTetrahedronOctahedronLatticePositionsWithinBoundingCube(const glm::vec3& boundingCube, const float& latticeSize)
        {
            
            glm::vec3 v1(1., 0., 0.);
            glm::vec3 v2(0.5, 0.5 * sqrt(3.), 0.);
            glm::vec3 v3(0.5, sqrt(3.)/6., sqrt(2./3.));
            
            return fillBoundingCubeWithLattice(v1, v2, v3, boundingCube, latticeSize);
        }
        
        std::vector<glm::vec3> getShiftedCubeLatticePositionsWithinBoundingCube(const glm::vec3& boundingCube, const float& latticeSize)
        {
            glm::vec3 v1(1., 0., 0.);
            glm::vec3 v2(0, 1., 0.);
            glm::vec3 v3(0.5, 0.5, 0.5 * sqrt(2.));
            
            return fillBoundingCubeWithLattice(v1, v2, v3, boundingCube, latticeSize);
        }

    }
}
