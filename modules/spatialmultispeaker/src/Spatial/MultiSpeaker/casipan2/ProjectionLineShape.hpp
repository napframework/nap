//
//  ProjectionLineShape.hpp
//  4dengine
//
//  Created by Casimir Geelhoed on 25/07/2018.
//
//

#ifndef ProjectionLineShape_hpp
#define ProjectionLineShape_hpp

#include <stdio.h>
#include "Shape.hpp"
#include "Functions.hpp"

#include <algorithm>
#include <iterator>

#include <glm/vec2.hpp>

namespace casipan {

    class ProjectionLineShape : public Shape {
        
    public:
        
        // Looking from outside the grid towards the projection point:
        // Speaker[0] is the left speaker
        // Speaker[1] is the right speaker
        ProjectionLineShape(const std::vector<Speaker*>& speakers) : Shape(speakers) {
            
            assert(speakers.size() == 2);
            
            // speakers in 2D coordinates.
            speakerA = to2D(speakers[0]->position);
            speakerB = to2D(speakers[1]->position);
            
            // If we define dx=xB-xA and dy=yB-yA, then the normals are (-dy, dx) and (dy, -dx).
            // I'm interested only in (-dy, dx), the normal pointing outwards.
            float dx = speakerB.x - speakerA.x;
            float dy = speakerB.y - speakerA.y;
            outwardsNormal = glm::normalize(glm::vec2(-dy, dx));
            
            normalizedAtoB = glm::normalize(speakerB-speakerA);
            
        }
        
        void getAllAmplitudesForSource(const Source& source, const GridSettings& settings, Amplitudes& amps) override final {

            
            if(source.isPointSource()){
                
                glm::vec2 pos2D = to2D(source.position);
                glm::vec2 point;
                
                if(glm::dot(pos2D - speakerA, outwardsNormal) > 0.){
                    if(segmentsIntersect(pos2D, to2D(settings.projectionPoint), speakerA, speakerB)){
                        point = segmentIntersection(pos2D, to2D(settings.projectionPoint), speakerA, speakerB);
                    }
                    else{
                        return;
                    }
                }
                else{
                    return;
                }
                
                float point1D = glm::dot(point - speakerA, normalizedAtoB) / glm::distance(speakerA,speakerB);
                
                if(point1D >= -epsilon && point1D <= 1. + epsilon){
                    
                    // clip point (could be outside range because of epsilon)
                    if(point1D > 1.)
                        point1D = 1.;
                    else if(point1D < 0.)
                        point1D = 0.;
                    
                    amps.set(speakers[0]->channel, 1.0 - point1D);
                    amps.set(speakers[1]->channel, point1D);
                }

                
                
            }
            else{
            
                const auto& edges = source.get2DEdges();
                
                std::vector<glm::vec2> pointsList;
                
                const glm::vec2 projectionPoint = to2D(settings.projectionPoint);
                
                for(auto& edge : edges){
                    
                    bool firstPointIsOutside = glm::dot(edge.first - speakerA, outwardsNormal) > 0.;
                    bool secondPointIsOutside = glm::dot(edge.second - speakerA, outwardsNormal) > 0.;
                    
                    // if the edge crosses the line through the projection line segment (which means one of the edges is outside, one inside), add the intersection point.
                    if(firstPointIsOutside != secondPointIsOutside)
                        pointsList.push_back(segmentIntersection(edge.first, edge.second, speakerA, speakerB));
                        
                    // project the edges and add them if they are on the outerside of the plane. (even if the projection falls outside of the range).
                    if(firstPointIsOutside)
                        pointsList.push_back(segmentIntersection(edge.first, projectionPoint, speakerA, speakerB));

                    if(secondPointIsOutside)
                        pointsList.push_back(segmentIntersection(edge.second, projectionPoint, speakerA, speakerB));
                    
                    // (we will get some duplicate points because every vertex is part of multiple edges, could be optimized at some point).

                }
                
                // We now have a list of projected 2D points onto the line.
                // Let's convert these points to points on a 1D line where "0" is speakerA and "1" is speakerB:
                std::vector<float> pointsList1D;
                
                for(auto& point : pointsList){
                    pointsList1D.push_back(glm::dot(point - speakerA, normalizedAtoB) / glm::distance(speakerA,speakerB));
                }

                // if there are no points, just return without setting any amp.
                if(pointsList1D.empty())
                    return;
                
                
                // Now let's find the min and max to calculate the amplitude values for speakerA and speakerB.
                
                float minimum = *std::min_element(pointsList1D.begin(), pointsList1D.end());
                float maximum = *std::max_element(pointsList1D.begin(), pointsList1D.end());

                
                float ampA = 0., ampB = 0.;
                
                
                
                if(minimum <= 0.){
                    if(maximum >= 0.)
                        ampA = 1.0;
                }
                else if(minimum > 0. && minimum <= 1.){
                    ampA = 1.0 - minimum;
                }
                else{
                    ampA = 0.0;
                }
                
                if(maximum >= 1.){
                    if(minimum <= 1.)
                        ampB = 1.0;
                }
                else if(maximum < 1. && maximum >= 0.){
                    ampB = maximum;
                }
                else{
                    ampB = 0.0;
                }
                
                
                amps.set(speakers[0]->channel, ampA);
                amps.set(speakers[1]->channel, ampB);
                
            }
            
        }
        
        
        // checks if speakers could form a valid shape of this type.
        static bool isValid(const std::vector<Speaker*>& speakers){
            return speakers.size() == 2;
        }
        
    private:
        float epsilon = 0.0001;
        
        glm::vec2 speakerA, speakerB, outwardsNormal, normalizedAtoB;
    };
    
}


#endif /* ProjectionLineShape_hpp */
