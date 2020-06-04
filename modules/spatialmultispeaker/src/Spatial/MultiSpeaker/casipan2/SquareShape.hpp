//
//  SquareShape.hpp
//  4dengine
//
//  Created by Casimir Geelhoed on 25/07/2018.
//
//

#ifndef SquareShape_hpp
#define SquareShape_hpp

#include <stdio.h>

#include "Shape.hpp"
#include "Functions.hpp"


namespace casipan {

    class SquareShape : public Shape {
        
    public:
        
        SquareShape(const std::vector<Speaker*> speakers) : Shape(speakers) {
            
            minX = speakers[0]->position.x;
            minZ = speakers[0]->position.z;
            maxX = speakers[3]->position.x;
            maxZ = speakers[3]->position.z;
            
            lenX = maxX - minX;
            lenZ = maxZ - minZ;
            
        }
        

        void getAllAmplitudesForSource(const Source& source, const GridSettings& settings, Amplitudes& amps) override final {
            
            for(int i = 0; i < 4; i++){
                            
                glm::vec2 closestPointToSpeaker = source.getClosestPointTo2D(to2D(speakers[i]->position));
                
                if(isInside(closestPointToSpeaker)){
                    
                    //relative positions
                    float posX = closestPointToSpeaker.x - minX;
                    float posZ = closestPointToSpeaker.y - minZ;
                    
                    if(i == 0)
                        amps.set(speakers[0]->channel, ((lenX - posX) / lenX)  * ((lenZ - posZ) / lenZ));
                    else if(i == 1)
                        amps.set(speakers[1]->channel, ((posX / lenX)  * ((lenZ - posZ) / lenZ) ));
                    else if(i == 2)
                        amps.set(speakers[2]->channel, ((lenX - posX) / lenX) * (posZ / lenZ));
                    else if(i == 3)
                        amps.set(speakers[3]->channel, ((posX / lenX) * (posZ / lenZ)));
                    
                }
                
            }
            
        }
        
        
        static bool isValid(const std::vector<Speaker*>& speakers){
            
            if(speakers.size() != 4)
                return false;
            return true;
        }
        
    private:
        
        float epsilon = 0.0001;
        
        bool isInside(glm::vec2 position) {
            return position.x >= minX - epsilon && position.x <= maxX + epsilon
            && position.y >= minZ - epsilon && position.y <= maxZ + epsilon;
        }

        
        float minX, minZ, maxX, maxZ, lenX, lenZ;
        
    };

}


#endif /* SquareShape_hpp */
