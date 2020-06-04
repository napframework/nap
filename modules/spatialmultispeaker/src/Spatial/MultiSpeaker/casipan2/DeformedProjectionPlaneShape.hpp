//
//  DeformedProjectionPlaneShape.hpp
//  testframework
//
//  Created by Casimir Geelhoed on 29/08/2018.
//
//

#ifndef DeformedProjectionPlaneShape_hpp
#define DeformedProjectionPlaneShape_hpp

#include <stdio.h>
#include "Shape.hpp"
#include "Functions.hpp"

#include <algorithm>
#include <iterator>

#include <iostream>

#include <glm/vec2.hpp>


namespace casipan {

    class DeformedProjectionPlaneShape : public Shape {
        
    public:
        
        // Order, looking from outside of the grid:
        // 1 left below
        // 2 right below
        // 3 left above
        // 4 right above
        
        DeformedProjectionPlaneShape(const std::vector<Speaker*>& speakers) : Shape(speakers) {
            
            assert(speakers.size() == 4);
            
            outwardsNormal = glm::normalize(glm::cross(speakers[1]->position - speakers[0]->position, speakers[2]->position - speakers[0]->position));
            xAxis = glm::normalize(speakers[1]->position-speakers[0]->position);
            yAxis = -glm::cross(xAxis,outwardsNormal);

            
            // set speaker positions in plane coordinates
            speakerPositionsInPlaneCoordinates.push_back(glm::vec2(0,0));                               // speaker 0
            speakerPositionsInPlaneCoordinates.push_back(toPlaneCoordinates(speakers[1]->position, speakers[0]->position, xAxis, yAxis));    // speaker 1
            speakerPositionsInPlaneCoordinates.push_back(toPlaneCoordinates(speakers[2]->position, speakers[0]->position, xAxis, yAxis));    // speaker 2
            speakerPositionsInPlaneCoordinates.push_back(toPlaneCoordinates(speakers[3]->position, speakers[0]->position, xAxis, yAxis));    // speaker 3
            
            counterClockwiseSpeakerPositionsInPlaneCoordinates = {
                speakerPositionsInPlaneCoordinates[0],
                speakerPositionsInPlaneCoordinates[1],
                speakerPositionsInPlaneCoordinates[3],
                speakerPositionsInPlaneCoordinates[2]
            };
            
        }
        
        
        void getAllAmplitudesForSource(const Source& source, const GridSettings& settings, Amplitudes& amps) override final {
            
            
            if(source.isPointSource()){
                
                if(glm::dot(source.position - speakers[0]->position, outwardsNormal) > 0.){ // on the right side of the plane
                    
                    // intersect the ray through position
                    glm::vec3 projectionPosition = intersectLinePlane(source.position, settings.projectionPoint, speakers[0]->position, outwardsNormal);
                    
                    // we now have the intersection point in 3D. Convert to 2D and calculate the amplitude.
                    
                    glm::vec2 position2D = toPlaneCoordinates(projectionPosition, speakers[0]->position, xAxis, yAxis);
                    
                    // deformed square shape now
                    if(isInsidePolygon(position2D)){
                        
                        glm::vec2 relative2DPosition = mapQuadrilateralPointToRectanglePoint(position2D, speakerPositionsInPlaneCoordinates[0], speakerPositionsInPlaneCoordinates[1], speakerPositionsInPlaneCoordinates[2], speakerPositionsInPlaneCoordinates[3]);
                        float valX = clamp<float>(relative2DPosition.x, 0., 1.);
                        float valY = clamp<float>(relative2DPosition.y, 0., 1.);

                        amps.set(speakers[0]->channel, (1. - valX) * (1. - valY));
                        amps.set(speakers[1]->channel, valX * (1. - valY));
                        amps.set(speakers[2]->channel, (1. - valX) * valY);
                        amps.set(speakers[3]->channel, valX * valY);
                        
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
                for(int i = 0; i < 4; i++){
                    
                    // TODO possible bug here:
                    // There could be discrepancy between the 'closest point' and the 'point with highest amplitude'. Because of the deforming..
                    // The solution would be to first transform *every* polygon from quadrilateral points to rectangle points and then find the closest point to the speakers.
                    // Something to look into later.
                    
                    glm::vec2 closestPointToSpeaker = getClosestPointOnPolygons(speakerPositionsInPlaneCoordinates[i], clippedToPlanePolygons);
                    
                    glm::vec2 relative2DPosition = mapQuadrilateralPointToRectanglePoint(closestPointToSpeaker, speakerPositionsInPlaneCoordinates[0], speakerPositionsInPlaneCoordinates[1], speakerPositionsInPlaneCoordinates[2], speakerPositionsInPlaneCoordinates[3]);
                    
                    float valX = clamp<float>(relative2DPosition.x, 0., 1.);
                    float valY = clamp<float>(relative2DPosition.y, 0., 1.);
                    
                    amps.set(speakers[0]->channel, (1. - valX) * (1. - valY));
                    amps.set(speakers[1]->channel, valX * (1. - valY));
                    amps.set(speakers[2]->channel, (1. - valX) * valY);
                    amps.set(speakers[3]->channel, valX * valY);
                        
                }
                
            }
            
        }
        
        // checks if speakers could form a valid shape of this type.
        static bool isValid(const std::vector<Speaker*>& speakers){
            return speakers.size() == 4;
        }
        
    private:
        
        
        // checks if the position in plane coordinates falls within the plane.
        bool isInsidePolygon(glm::vec2 position) const{
            return pointInPoly(position, counterClockwiseSpeakerPositionsInPlaneCoordinates);
        }
        
        std::vector<glm::vec2> speakerPositionsInPlaneCoordinates;
        std::vector<glm::vec2> counterClockwiseSpeakerPositionsInPlaneCoordinates;
        glm::vec3 outwardsNormal;
        glm::vec3 xAxis;
        glm::vec3 yAxis;
        
    };
    
}

#endif /* DeformedProjectionPlaneShape_hpp */
