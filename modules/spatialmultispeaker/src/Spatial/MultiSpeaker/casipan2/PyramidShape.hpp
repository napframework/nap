//
//  PyramidShape.hpp
//  testframework
//
//  Created by Casimir Geelhoed on 29/08/2018.
//
//

#ifndef PyramidShape_hpp
#define PyramidShape_hpp

#include "Shape.hpp"

namespace casipan {

    class PyramidShape : public Shape {
        
    public:
        
        // 0 is [minX, minZ]
        // 1 is [maxX, minZ]
        // 2 is [minX, maxZ]
        // 3 is [maxX, maxZ]
        // 4 is top
        
        PyramidShape(const std::vector<Speaker*>& speakers) : Shape(speakers) {
            
            assert(speakers.size() == 5);
            
            inwardsPlaneNormal = -glm::normalize(glm::cross(speakers[0]->position - speakers[1]->position, speakers[0]->position - speakers[2]->position));
            
            xAxis = glm::normalize(speakers[1]->position-speakers[0]->position);
            yAxis = glm::normalize(speakers[2]->position-speakers[0]->position);
            
            xMax = glm::distance(speakers[1]->position,speakers[0]->position);
            yMax = glm::distance(speakers[2]->position,speakers[0]->position);
            
        }
        
        
        void getAllAmplitudesForSource(const Source& source, const GridSettings& settings, Amplitudes& amps) override final {
            
            
            for(int i = 0; i < 5; i++){
                
                float distance;
                glm::vec3 closestPointToSpeaker = source.getClosestPointTo(speakers[i]->position, distance);
                
                
                // check if position is on right side of ground plane
                if(glm::dot(closestPointToSpeaker - speakers[0]->position, inwardsPlaneNormal) > -epsilon){
                    
                    // check if position is not further away then top (if the line from topspeaker to point is pointed in opposite direction of planeNormal)
                    if(glm::dot(closestPointToSpeaker - speakers[4]->position, inwardsPlaneNormal) < epsilon){
                        
                        // project point onto plane
                        glm::vec3 projectedPoint = intersectLinePlane(speakers[4]->position, closestPointToSpeaker, speakers[0]->position, inwardsPlaneNormal);
                        
                        // convert to 2D coordinates
                        glm::vec2 position2D = toPlaneCoordinates(projectedPoint, speakers[0]->position, xAxis, yAxis);
                        
                        // check if inside poly
                        if(isInsideSquare(position2D)){
                            
                            
                            // calculate values for height, width and length and set amps accordingly.
                            
                            float heightValue = 1. - (glm::distance(speakers[4]->position,closestPointToSpeaker) /  glm::distance(speakers[4]->position, projectedPoint));
                            heightValue = glm::clamp<float>(heightValue, 0., 1.);
                            
                            if(i == 4){
                                
                                amps.set(speakers[4]->channel, heightValue);
                                
                            }
                            else{
                                
                                float widthValue  = glm::clamp<float>(position2D.x / xMax, 0., 1.);
                                float lengthValue = glm::clamp<float>(position2D.y / yMax, 0., 1.);
                                
                                if(i == 0)
                                    amps.set(speakers[0]->channel, (1. - widthValue) * (1. - lengthValue) * (1. - heightValue));
                                else if(i == 1)
                                    amps.set(speakers[1]->channel, (widthValue) * (1. - lengthValue) * (1. - heightValue));
                                else if(i == 2)
                                    amps.set(speakers[2]->channel, (1. - widthValue) * (lengthValue) * (1. - heightValue));
                                else if(i == 3)
                                    amps.set(speakers[3]->channel, (widthValue) * (lengthValue) * (1. - heightValue));
                                
                            }
                        }
                    }
                }
            }
        }
        
        static bool isValid(const std::vector<Speaker*>& speakers){
            
            if(speakers.size() != 5)
                return false;
            
            return true;
        }
        
    private:
        
        float epsilon = 0.0001;
        
        inline bool isInsideSquare(const glm::vec2& position) const{
            return position.x >= - epsilon && position.x <= xMax + epsilon
            && position.y >= - epsilon && position.y <= yMax + epsilon;
        }
        
        glm::vec3 inwardsPlaneNormal;
        glm::vec3 xAxis;
        glm::vec3 yAxis;
        float xMax, yMax;
    };

}

#endif /* PyramidShape_hpp */
