//
//  TetrahedronShape.hpp
//  theworks
//
//  Created by Casimir Geelhoed on 27/11/2018.
//
//

#ifndef TetrahedronShape_hpp
#define TetrahedronShape_hpp

#include <stdio.h>

#include "Shape.hpp"

namespace casipan {

    class TetrahedronShape : public Shape {
        
    public:
        
        TetrahedronShape(const std::vector<Speaker*> speakers) : Shape(speakers) {
            
            assert(speakers.size() == 4);
            
            totalVolume = getVolume(speakers[0]->position, speakers[1]->position, speakers[2]->position, speakers[3]->position);
        }
        
        
        void getAllAmplitudesForSource(const Source& source, const GridSettings& settings, Amplitudes& amps) override final {
            
            
            for(int i = 0; i < 4; i++){
                
                float distance;
                glm::vec3 closestPointToSpeaker = source.getClosestPointTo(speakers[i]->position, distance);
                
                if(isInside(closestPointToSpeaker)){
                    
                    if(i == 0)
                        amps.set(speakers[0]->channel, getVolume(closestPointToSpeaker, speakers[1]->position, speakers[2]->position, speakers[3]->position)/totalVolume);
                    else if(i == 1)
                        amps.set(speakers[1]->channel, getVolume(closestPointToSpeaker, speakers[2]->position, speakers[3]->position, speakers[0]->position)/totalVolume);
                    else if(i == 2)
                        amps.set(speakers[2]->channel, getVolume(closestPointToSpeaker, speakers[3]->position, speakers[0]->position, speakers[1]->position)/totalVolume);
                    else if(i == 3)
                        amps.set(speakers[3]->channel, getVolume(closestPointToSpeaker, speakers[0]->position, speakers[1]->position, speakers[2]->position)/totalVolume);
                    
                }
                
            }
            
        }

        static bool isValid(const std::vector<Speaker*>& speakers){
            
            if(speakers.size() != 4)
                return false;
            
            return true;
        }
        
    private:
        
        float totalVolume;
        
        bool isInside(glm::vec3 position){
            return  SameSide(speakers[0]->position, speakers[1]->position, speakers[2]->position, speakers[3]->position, position) &&
            SameSide(speakers[1]->position, speakers[2]->position, speakers[3]->position, speakers[0]->position, position) &&
            SameSide(speakers[2]->position, speakers[3]->position, speakers[0]->position, speakers[1]->position, position) &&
            SameSide(speakers[3]->position, speakers[0]->position, speakers[1]->position, speakers[2]->position, position);
        }

        
        // https://stackoverflow.com/a/12516553/1177065
        float getVolume(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 d){
            return (1./6.) * glm::length(glm::determinant(glm::mat3(a-d,b-d,c-d)));
        }
        
        bool SameSide(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3, glm::vec3 v4, glm::vec3 p)
        {
            glm::vec3 normal = glm::cross(v2 - v1, v3 - v1);
            float dotV4 = glm::dot(normal, v4 - v1);
            float dotP = glm::dot(normal, p - v1);
            return (dotV4 >= 0 && dotP >= 0) || (dotV4 <= 0 && dotP <= 0);
        }
        
    };
    
}



#endif /* TetrahedronShape_hpp */
