//
//  TriangularPrismShape.hpp
//  theworks
//
//  Created by Casimir Geelhoed on 28/11/2018.
//
//

#ifndef TriangularPrismShape_hpp
#define TriangularPrismShape_hpp

#include <stdio.h>

#include "Shape.hpp"
#include "Functions.hpp"


namespace casipan {

    class TriangularPrismShape : public Shape {
        
    public:
        
        enum TrianglePlaneOrientation { XY, XZ, YZ };
        
        
        // speakers 0,1,2 form a triangle on x-z plane, x-y plane or y-z plane
        // speakers 3,4,5 form the same triangle, but offset in positive x-axis (when triangle is on y-z plane), y-axis (when x-z plane) or z-axis (when x-y plane).
        
        TriangularPrismShape(const std::vector<Speaker*> speakers) : Shape(speakers) {
            
            assert(speakers.size() == 6);
            
            
            if(fabs(speakers[0]->position.z - speakers[1]->position.z) < 0.0001 &&
               fabs(speakers[0]->position.z - speakers[2]->position.z) < 0.0001){
                trianglePlaneOrientation = XY;
            }
            else if(fabs(speakers[0]->position.y - speakers[1]->position.y) < 0.0001 &&
                    fabs(speakers[0]->position.y - speakers[2]->position.y) < 0.0001){
                trianglePlaneOrientation = XZ;
            }
            else{
                trianglePlaneOrientation = YZ;

            }
            
            
            if(trianglePlaneOrientation == XY){
                speakers2D.push_back(glm::vec2(speakers[0]->position.x,speakers[0]->position.y));
                speakers2D.push_back(glm::vec2(speakers[1]->position.x,speakers[1]->position.y));
                speakers2D.push_back(glm::vec2(speakers[2]->position.x,speakers[2]->position.y));
                minHeight = speakers[0]->position.z;
                maxHeight = speakers[4]->position.z;

            }
            else if(trianglePlaneOrientation == XZ){
                speakers2D.push_back(glm::vec2(speakers[0]->position.x,speakers[0]->position.z));
                speakers2D.push_back(glm::vec2(speakers[1]->position.x,speakers[1]->position.z));
                speakers2D.push_back(glm::vec2(speakers[2]->position.x,speakers[2]->position.z));
                minHeight = speakers[0]->position.y;
                maxHeight = speakers[4]->position.y;
            }
            else{ // YZ
                speakers2D.push_back(glm::vec2(speakers[0]->position.y,speakers[0]->position.z));
                speakers2D.push_back(glm::vec2(speakers[1]->position.y,speakers[1]->position.z));
                speakers2D.push_back(glm::vec2(speakers[2]->position.y,speakers[2]->position.z));
                minHeight = speakers[0]->position.x;
                maxHeight = speakers[4]->position.x;
            }
            
            totalArea = triangleArea(speakers2D[0], speakers2D[1], speakers2D[2]);
            
        }
        
        
        void getAllAmplitudesForSource(const Source& source, const GridSettings& settings, Amplitudes& amps) override final {
            
            for(int i = 0; i < 6; i++){
                
                float distance;
                glm::vec3 closestPointToSpeaker = source.getClosestPointTo(speakers[i]->position, distance);
                
                if(isInside(closestPointToSpeaker)){
                    
                    float heightValue;
                    glm::vec2 pos;
                    
                    if(trianglePlaneOrientation == XY){
                        heightValue = (closestPointToSpeaker.z - minHeight) / (maxHeight - minHeight);
                        pos = glm::vec2(closestPointToSpeaker.x, closestPointToSpeaker.y);
                    }
                    else if(trianglePlaneOrientation == XZ){
                        heightValue = (closestPointToSpeaker.y - minHeight) / (maxHeight - minHeight);
                        pos = glm::vec2(closestPointToSpeaker.x, closestPointToSpeaker.z);
                    }
                    else{ // YZ
                        heightValue = (closestPointToSpeaker.x - minHeight) / (maxHeight - minHeight);
                        pos = glm::vec2(closestPointToSpeaker.y, closestPointToSpeaker.z);
                    }
                    
                    heightValue = clamp<float>(heightValue, 0., 1.);
                    
                    if(i == 0)
                        amps.set(speakers[0]->channel, triangleArea(pos, speakers2D[1], speakers2D[2])/totalArea * (1 - heightValue));
                    else if(i == 1)
                        amps.set(speakers[1]->channel, triangleArea(pos, speakers2D[2], speakers2D[0])/totalArea * (1 - heightValue));
                    else if(i == 2)
                        amps.set(speakers[2]->channel, triangleArea(pos, speakers2D[0], speakers2D[1])/totalArea * (1 - heightValue));
                    else if(i == 3)
                        amps.set(speakers[3]->channel, triangleArea(pos, speakers2D[1], speakers2D[2])/totalArea * heightValue);
                    else if(i == 4)
                        amps.set(speakers[4]->channel, triangleArea(pos, speakers2D[2], speakers2D[0])/totalArea * heightValue);
                    else if(i == 5)
                        amps.set(speakers[5]->channel, triangleArea(pos, speakers2D[0], speakers2D[1])/totalArea * heightValue);
                        
                }
                
            }
            
        }
        
        static bool isValid(const std::vector<Speaker*>& speakers){
            
            if(speakers.size() != 6)
                return false;
            
            return true;
        }
        
    private:
        std::vector<glm::vec2> speakers2D;
        float minHeight, maxHeight;
        float totalArea;
        
        TrianglePlaneOrientation trianglePlaneOrientation;
        
        bool isInside(glm::vec3 position){
            if(trianglePlaneOrientation == XY)
                return pointInPoly(glm::vec2(position.x, position.y), speakers2D) && position.z >= minHeight && position.z <= maxHeight;
            else if(trianglePlaneOrientation == XZ)
                return pointInPoly(glm::vec2(position.x, position.z), speakers2D) && position.y >= minHeight && position.y <= maxHeight;
            else
                return pointInPoly(glm::vec2(position.y, position.z), speakers2D) && position.x >= minHeight && position.x <= maxHeight;
        }
        
    };
    
}


#endif /* TriangularPrismShape_hpp */
