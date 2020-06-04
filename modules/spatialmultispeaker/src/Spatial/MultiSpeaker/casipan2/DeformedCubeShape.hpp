//
//  DeformedCubeShape.hpp
//  testframework
//
//  Created by Casimir Geelhoed on 29/08/2018.
//
//

#ifndef DeformedCubeShape_hpp
#define DeformedCubeShape_hpp

#include <stdio.h>


#include "Shape.hpp"
#include "Functions.hpp"

namespace casipan {

    class DeformedCubeShape : public Shape {
        
    public:
        
        // 0 is [minX, minY, minZ]
        // 1 is [maxX, minY, minZ]
        // 2 is [minX, maxY, minZ]
        // 3 is [maxX, maxY, minZ]
        // 4 is [minX, minY, maxZ]
        // 5 is [maxX, minY, maxZ]
        // 6 is [minX, maxY, maxZ]
        // 7 is [maxX, maxY, maxZ]
        // Deformation on the xz plane is allowed.
        DeformedCubeShape(const std::vector<Speaker*>& speakers) : Shape(speakers) {
            
            assert(speakers.size() == 8);
            
            minX = speakers[0]->position.x;
            minY = speakers[0]->position.y;
            minZ = speakers[0]->position.z;
            maxX = speakers[7]->position.x;
            maxY = speakers[7]->position.y;
            maxZ = speakers[7]->position.z;
            
            lenX = maxX - minX;
            lenY = maxY - minY;
            lenZ = maxZ - minZ;
            
            speakerPositions2D.push_back(to2D(speakers[0]->position));
            speakerPositions2D.push_back(to2D(speakers[4]->position));
            speakerPositions2D.push_back(to2D(speakers[5]->position));
            speakerPositions2D.push_back(to2D(speakers[1]->position));
            
            
        }
        
        
        void getAllAmplitudesForSource(const Source& source, const GridSettings& settings, Amplitudes& amps) override final {
            
            for(int i = 0; i < 8; i++){
                
                glm::vec3 closestPointToSpeaker = source.getClosestPointTo(speakers[i]->position);
                
                if(isInside(closestPointToSpeaker)){
                    
                    glm::vec2 relative2DPosition = mapQuadrilateralPointToRectanglePoint(closestPointToSpeaker, speakerPositions2D[0], speakerPositions2D[1], speakerPositions2D[2], speakerPositions2D[3]);
                    
                    float valX = clamp<float>(relative2DPosition.x, 0., 1.);
                    float valY = clamp<float>((closestPointToSpeaker.y - speakers[0]->position.y) / lenY, 0., 1.);
                    float valZ = clamp<float>(relative2DPosition.y, 0., 1.);

                    if(i == 0)
                        amps.set(speakers[0]->channel, (1. - valX) * (1. - valY) * (1. - valZ));
                    else if(i == 1)
                        amps.set(speakers[1]->channel, (1. - valX) * (1. - valY) * valZ);
                    else if(i == 2)
                        amps.set(speakers[2]->channel, (1. - valX) * valY * (1. - valZ));
                    else if(i == 3)
                        amps.set(speakers[3]->channel, (1. - valX) * valY * valZ);
                    else if(i == 4)
                        amps.set(speakers[4]->channel, valX * (1. - valY) * (1. - valZ));
                    else if(i == 5)
                        amps.set(speakers[5]->channel, valX * (1. - valY) * valZ);
                    else if(i == 6)
                        amps.set(speakers[6]->channel, valX * valY * (1. - valZ));
                    else if(i == 7)
                        amps.set(speakers[7]->channel, valX * valY * valZ);
                    
                }
                
            }
            
        }
        
        
        // Checks if speakers could form a valid shape of this type.
        static bool isValid(const std::vector<Speaker*>& speakers){
            
            return speakers.size() == 8
            
            // check if y positions are the same for both x,z quadrilaterals
            && speakers[0]->position.y == speakers[1]->position.y
            && speakers[0]->position.y == speakers[4]->position.y
            && speakers[0]->position.y == speakers[5]->position.y
            && speakers[2]->position.y == speakers[3]->position.y
            && speakers[2]->position.y == speakers[6]->position.y
            && speakers[2]->position.y == speakers[7]->position.y
            
            // check if speakers 0,1,4,5 are below speakers 2,3,6,7
            && speakers[0]->position.y < speakers[2]->position.y
            && speakers[1]->position.y < speakers[3]->position.y
            && speakers[4]->position.y < speakers[6]->position.y
            && speakers[5]->position.y < speakers[7]->position.y
            
            // check if x,z coordinates align of all columns
            && speakers[0]->position.x == speakers[2]->position.x
            && speakers[0]->position.z == speakers[2]->position.z
            && speakers[1]->position.x == speakers[3]->position.x
            && speakers[1]->position.z == speakers[3]->position.z
            && speakers[4]->position.x == speakers[6]->position.x
            && speakers[4]->position.z == speakers[6]->position.z
            && speakers[5]->position.x == speakers[7]->position.x
            && speakers[5]->position.z == speakers[7]->position.z;
            
        }
        
    private:
        
        float epsilon = 0.0001;
        
        
        inline bool isInside(glm::vec3 position) const {
            return pointInPoly(to2D(position), speakerPositions2D) && position.y >= minY - epsilon && position.y <= maxY + epsilon;
        }
        
        std::vector<glm::vec2> speakerPositions2D;
        float minX, minY, minZ, maxX, maxY, maxZ, lenX, lenY, lenZ;
        
    };
    
}

#endif /* DeformedCubeShape_hpp */
