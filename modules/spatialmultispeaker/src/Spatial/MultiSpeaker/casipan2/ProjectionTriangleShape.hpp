//
//  ProjectionTriangleShape.hpp
//  testframework
//
//  Created by Casimir Geelhoed on 29/08/2018.
//
//

#ifndef ProjectionTriangleShape_hpp
#define ProjectionTriangleShape_hpp

#include <stdio.h>
#include "Shape.hpp"
#include "Functions.hpp"

#include <algorithm>
#include <iterator>

#include <iostream>

#include <glm/vec2.hpp>


namespace casipan {

    class ProjectionTriangleShape : public Shape {
        
    public:
        
        // Speakers in counterclockwise order, looking from outside of the grid.
        
        ProjectionTriangleShape(const std::vector<Speaker*>& speakers) : Shape(speakers) {
            
            outwardsNormal = glm::normalize(glm::cross(speakers[1]->position - speakers[0]->position, speakers[2]->position - speakers[0]->position));
            xAxis = glm::normalize(speakers[1]->position-speakers[0]->position);
            yAxis = -glm::cross(xAxis,outwardsNormal);
            
            
            // set speaker positions in plane coordinates
            speakerPositionsInPlaneCoordinates.push_back(glm::vec2(0,0));                                                                    // speaker 0
            speakerPositionsInPlaneCoordinates.push_back(toPlaneCoordinates(speakers[1]->position, speakers[0]->position, xAxis, yAxis));    // speaker 1
            speakerPositionsInPlaneCoordinates.push_back(toPlaneCoordinates(speakers[2]->position, speakers[0]->position, xAxis, yAxis));    // speaker 2
            
            counterClockwiseSpeakerPositionsInPlaneCoordinates = {
                speakerPositionsInPlaneCoordinates[0],
                speakerPositionsInPlaneCoordinates[1],
                speakerPositionsInPlaneCoordinates[2]
            };
            
            totalArea = triangleArea(speakerPositionsInPlaneCoordinates[0],speakerPositionsInPlaneCoordinates[1],speakerPositionsInPlaneCoordinates[2]);
            
        }
        
        
        void getAllAmplitudesForSource(const Source& source, const GridSettings& settings, Amplitudes& amps) override final {
            
            
            if(source.isPointSource()){
                
                if(glm::dot(source.position - speakers[0]->position, outwardsNormal) > 0.){ // on the right side of the plane
                    
                    // intersect the ray through position
                    glm::vec3 projectionPosition = intersectLinePlane(source.position, settings.projectionPoint, speakers[0]->position, outwardsNormal);
                    
                    // we now have the intersection point in 3D. Convert to 2D and calculate the amplitude.
                    
                    glm::vec2 position2D = toPlaneCoordinates(projectionPosition, speakers[0]->position, xAxis, yAxis);
                    
                    if(isInsidePolygon(position2D)){
                        
                        amps.set(speakers[0]->channel, triangleArea(position2D, speakerPositionsInPlaneCoordinates[1], speakerPositionsInPlaneCoordinates[2])/totalArea);
                        amps.set(speakers[1]->channel, triangleArea(position2D, speakerPositionsInPlaneCoordinates[2], speakerPositionsInPlaneCoordinates[0])/totalArea);
                        amps.set(speakers[2]->channel, triangleArea(position2D, speakerPositionsInPlaneCoordinates[0], speakerPositionsInPlaneCoordinates[1])/totalArea);
                
                    }
                }
                
            }
            else{
                
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
                for(int i = 0; i < 3; i++){
                    
                    
                    glm::vec2 closestPointToSpeaker = getClosestPointOnPolygons(speakerPositionsInPlaneCoordinates[i], clippedToPlanePolygons);
                    
                    if(i == 0)
                        amps.set(speakers[0]->channel, triangleArea(closestPointToSpeaker, speakerPositionsInPlaneCoordinates[1], speakerPositionsInPlaneCoordinates[2])/totalArea);
                    else if(i == 1)
                        amps.set(speakers[1]->channel, triangleArea(closestPointToSpeaker, speakerPositionsInPlaneCoordinates[2], speakerPositionsInPlaneCoordinates[0])/totalArea);
                    else if(i == 2)
                        amps.set(speakers[2]->channel, triangleArea(closestPointToSpeaker, speakerPositionsInPlaneCoordinates[0], speakerPositionsInPlaneCoordinates[1])/totalArea);
                    
                }
                
            }
            
        }
        
        // checks if speakers could form a valid shape of this type.
        static bool isValid(const std::vector<Speaker*>& speakers){
            return speakers.size() == 3;
        }
        
    private:
        
        
        // checks if the position in plane coordinates falls within the plane.
        inline bool isInsidePolygon(const glm::vec2& position) const{
            return pointInPoly(position, counterClockwiseSpeakerPositionsInPlaneCoordinates);
        }
        
        std::vector<glm::vec2> speakerPositionsInPlaneCoordinates;
        std::vector<glm::vec2> counterClockwiseSpeakerPositionsInPlaneCoordinates;
        glm::vec3 outwardsNormal;
        glm::vec3 xAxis;
        glm::vec3 yAxis;
        
        float totalArea;
        
    };
    
}

#endif /* ProjectionTriangleShape_hpp */
