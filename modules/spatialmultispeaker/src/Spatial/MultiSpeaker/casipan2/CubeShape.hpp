//
//  CubeShape.hpp
//  4dengine
//
//  Created by Casimir Geelhoed on 11/07/2018.
//
//

#ifndef CubeShape_hpp
#define CubeShape_hpp

#include <stdio.h>

#include "Shape.hpp"

namespace casipan {

    class CubeShape : public Shape {
        
    public:
        
        // 0 is [minX, minY, minZ]
        // 1 is [maxX, minY, minZ]
        // 2 is [minX, maxY, minZ]
        // 3 is [maxX, maxY, minZ]
        // 4 is [minX, minY, maxZ]
        // 5 is [maxX, minY, maxZ]
        // 6 is [minX, maxY, maxZ]
        // 7 is [maxX, maxY, maxZ]
        // Tilted cubes are allowed.
        CubeShape(const std::vector<Speaker*>& speakers) : Shape(speakers) {
            
            assert(speakers.size() == 8);
            
            xAxis = glm::normalize(speakers[1]->position-speakers[0]->position);
            yAxis = glm::normalize(speakers[2]->position-speakers[0]->position);
            zAxis = glm::normalize(speakers[4]->position-speakers[0]->position);

            lenX = glm::distance(speakers[1]->position, speakers[0]->position);
            lenY = glm::distance(speakers[2]->position, speakers[0]->position);
            lenZ = glm::distance(speakers[4]->position, speakers[0]->position);
            
        }
        
            
        void getAllAmplitudesForSource(const Source& source, const GridSettings& settings, Amplitudes& amps) override final {
            
            
            for(int i = 0; i < 8; i++){
                
                // get closest point for speaker
                const glm::vec3 closestPointToSpeaker = source.getClosestPointTo(speakers[i]->position);
                
                // get relative position of closest point
                glm::vec3 closesPointMinusSpeaker0 = closestPointToSpeaker - speakers[0]->position;
                const float posX = glm::dot(closesPointMinusSpeaker0, xAxis);
                if(posX > lenX + epsilon || posX < -epsilon)
                    continue;
                const float posY = glm::dot(closesPointMinusSpeaker0, yAxis);
                if(posY > lenY + epsilon || posY < -epsilon)
                    continue;
                const float posZ = glm::dot(closesPointMinusSpeaker0, zAxis);
                if(posZ > lenZ + epsilon || posZ < -epsilon)
                    continue;

                // 3D interpolation
                if(i == 0)
                    amps.set(speakers[0]->channel, ((lenX - posX) / lenX) * ((lenY - posY) / lenY) * ((lenZ - posZ) / lenZ));
                else if(i == 1)
                    amps.set(speakers[1]->channel, ((posX / lenX) * ((lenY - posY) / lenY) * ((lenZ - posZ) / lenZ) ));
                else if(i == 2)
                    amps.set(speakers[2]->channel, ((lenX - posX) / lenX) * (posY / lenY) * ((lenZ - posZ) / lenZ));
                else if(i == 3)
                    amps.set(speakers[3]->channel, ((posX / lenX) * (posY / lenY) * ((lenZ - posZ) / lenZ)));
                else if(i == 4)
                    amps.set(speakers[4]->channel, ((lenX - posX) / lenX) * ((lenY - posY) / lenY) * (posZ / lenZ));
                else if(i == 5)
                    amps.set(speakers[5]->channel, ((posX / lenX) * ((lenY - posY) / lenY) * (posZ / lenZ)));
                else if(i == 6)
                    amps.set(speakers[6]->channel, ((lenX - posX) / lenX) * (posY / lenY) * (posZ / lenZ));
                else if(i == 7)
                    amps.set(speakers[7]->channel, ((posX / lenX) * (posY / lenY) * (posZ / lenZ)));
                    
            }
            
        }
        
        static bool isValid(const std::vector<Speaker*>& speakers){
            return speakers.size() == 8;
        }
		
	private:
	
		float epsilon = 0.0001;
	
		float lenX, lenY, lenZ;
        glm::vec3 xAxis;
        glm::vec3 yAxis;
        glm::vec3 zAxis;
		
    };

}

#endif /* CubeShape_hpp */
