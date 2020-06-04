//
//  ProjectionPlaneShape.hpp
//  testframework
//
//  Created by Casimir Geelhoed on 14/08/2018.
//
//

#ifndef ProjectionPlaneShape_hpp
#define ProjectionPlaneShape_hpp

#include <stdio.h>
#include "Shape.hpp"
#include "Functions.hpp"

#include <algorithm>
#include <iterator>

#include <iostream>

#include <glm/vec2.hpp>

namespace casipan {


    class ProjectionPlaneShape : public Shape {
        
    public:
        
        // Order, looking from outside of the grid:
        // 1 left below
        // 2 right below
        // 3 left above
        // 4 right above
        
        ProjectionPlaneShape(const std::vector<Speaker*>& speakers) : Shape(speakers) {
            
            assert(speakers.size() == 4);
            
            outwardsNormal = glm::normalize(glm::cross(speakers[1]->position - speakers[0]->position, speakers[2]->position - speakers[0]->position));
            xAxis = glm::normalize(speakers[1]->position-speakers[0]->position);
            yAxis = glm::normalize(speakers[2]->position-speakers[0]->position);
            
            // note : use:
            //  yAxis = -glm::cross(xAxis,planeNormal); for other shapes (that are not a perfect square)
            
            lenX = glm::length(speakers[1]->position-speakers[0]->position);
            lenY = glm::length(speakers[2]->position-speakers[0]->position);

            // set speaker positions in plane coordinates
            speakerPositionsInPlaneCoordinates.push_back(glm::vec2(0,0));       // speaker 0
            speakerPositionsInPlaneCoordinates.push_back(glm::vec2(lenX,0));    // speaker 1
            speakerPositionsInPlaneCoordinates.push_back(glm::vec2(0,lenY));    // speaker 2
            speakerPositionsInPlaneCoordinates.push_back(glm::vec2(lenX,lenY)); // speaker 3
            
            counterClockwiseSpeakerPositionsInPlaneCoordinates = {
                speakerPositionsInPlaneCoordinates[0],
                speakerPositionsInPlaneCoordinates[1],
                speakerPositionsInPlaneCoordinates[3],
                speakerPositionsInPlaneCoordinates[2]
            };

        }
        

        void getAllAmplitudesForSource(const Source& source, const GridSettings& settings, Amplitudes& amps) override final {

            
            if(source.isPointSource()){
                glm::vec2 point;
                
                if(glm::dot(source.position - speakers[0]->position, outwardsNormal) > 0.){ // on the right side of the plane
                    
                    // intersect the ray through position
                    glm::vec3 projectionPosition = intersectLinePlane(source.position, settings.projectionPoint, speakers[0]->position, outwardsNormal);
                    
                    // we now have the intersection point in 3D. Convert to 2D and calculate the amplitude.
                    
                    glm::vec2 position2D = toPlaneCoordinates(projectionPosition, speakers[0]->position, xAxis, yAxis);

                    // basically a square shape now.
                    
                    float posX = position2D.x;
                    float posY = position2D.y;
                    
                    if(isInsidePlane(position2D)){
                        // note: possible optimization: somehow check earlier whether a source falls within the frustrum.
                        
                        amps.set(speakers[0]->channel, ((lenX - posX) / lenX) * ((lenY - posY) / lenY));
                        amps.set(speakers[1]->channel, ((posX       ) / lenX) * ((lenY - posY) / lenY));
                        amps.set(speakers[2]->channel, ((lenX - posX) / lenX) * (posY / lenY));
                        amps.set(speakers[3]->channel, ((posX         / lenX) * (posY / lenY)));
                        
                    }
                }
                
            }
            else{
                
                // TODO Optimisation: vectors > arrays
                // TODO Optimisation: source.getPlanes() instead of source.getTriangles()
                
                const auto& polys = source.getTriangles();
                std::vector<std::vector<glm::vec2>> clippedToPlanePolygons;
                
                for(auto& poly : polys){
                    
                    std::vector<glm::vec2> clippedPoly = projectAndClip3DPolygonTo2DPolygon(poly, counterClockwiseSpeakerPositionsInPlaneCoordinates, outwardsNormal, speakers[0]->position, xAxis, yAxis, glm::normalize(settings.projectionPoint - source.position), settings.projectionPoint, settings.orthogonalProjection);
                    
                    
                    if(clippedPoly.size() > 0)
                        clippedToPlanePolygons.push_back(clippedPoly);
                        
                }
                    
                // return if nothing falls within the plane
                if(clippedToPlanePolygons.size() == 0)
                    return;

                // else, for each speaker, we calculate the closest point (in 2D 'plane'-space) and we set its amplitude accordingly.
                for(int i = 0; i < 4; i++){
                    
                    glm::vec2 closestPointToSpeaker = getClosestPointOnPolygons(speakerPositionsInPlaneCoordinates[i], clippedToPlanePolygons);

                    // clip values (it could be a little out of bounds because of floating point errors)
                    float posX = clamp<float>(closestPointToSpeaker.x, 0., lenX);
                    float posY = clamp<float>(closestPointToSpeaker.y, 0., lenY);
                    
                    // set amps
                    if(i == 0)
                        amps.set(speakers[0]->channel, ((lenX - posX) / lenX)  * ((lenY - posY) / lenY));
                    else if(i == 1)
                        amps.set(speakers[1]->channel, ((posX / lenX)  * ((lenY - posY) / lenY) ));
                    else if(i == 2)
                        amps.set(speakers[2]->channel, ((lenX - posX) / lenX) * (posY / lenY));
                    else if(i == 3)
                        amps.set(speakers[3]->channel, ((posX / lenX) * (posY / lenY)));
                }
                 
                
            }

        }
            
        // checks if speakers could form a valid shape of this type.
        static bool isValid(const std::vector<Speaker*>& speakers){
            return speakers.size() == 4;
        }
            
    private:
        
        float epsilon = 0.0001;
        
        // checks if the position in plane coordinates falls within the plane.
        inline bool isInsidePlane(const glm::vec2& position) const{
            return position.x >= - epsilon && position.x <= lenX + epsilon
            && position.y >= - epsilon && position.y <= lenY + epsilon;
        }
        
        std::vector<glm::vec2> speakerPositionsInPlaneCoordinates;
        std::vector<glm::vec2> counterClockwiseSpeakerPositionsInPlaneCoordinates;
        glm::vec3 outwardsNormal;
        glm::vec3 xAxis;
        glm::vec3 yAxis;
        float lenX, lenY;
            
    };
    
}


#endif /* ProjectionPlaneShape_hpp */
