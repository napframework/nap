//
//  CircleShape.hpp
//  Project
//
//  Created by Casimir Geelhoed on 09/10/2019.
//
//

#ifndef CircleShape_hpp
#define CircleShape_hpp

#include "TriangleShape.hpp"

#include "Shape.hpp"
#include "Functions.hpp"

#include <stdio.h>

namespace casipan {
    
    
    /**
     * A 'circle' shape consisting of multiple triangle shapes, consisting of 2 neighbouring speakers and a 'phantom speaker' in the center. The amplitude of the phantom speaker is distributed evenly over all speakers of the shape.
     */
    class CircleShape : public Shape {
        
    public:
        
        CircleShape(const std::vector<Speaker*>& speakers) : Shape(speakers) {
            
            // convert to 2D
            for(auto* speaker : speakers)
                speakerPositions2D.push_back(to2D(speaker->position));
            
            // calculate center
            glm::vec2 centerPosition;
            for(auto& speakerPosition : speakerPositions2D)
                centerPosition += speakerPosition;
            centerPosition /= speakerPositions2D.size();
            
            // create triangles
            triangles.reserve(speakerPositions2D.size());
            for(int i = 0; i < speakerPositions2D.size(); ++i){

                std::vector<glm::vec2> trianglePositions = { speakerPositions2D[i], speakerPositions2D[(i+1) % speakerPositions2D.size()], centerPosition};
                
                triangles.push_back(Triangle(trianglePositions));
            }
            
        }
        
        
        void getAllAmplitudesForSource(const Source& source, const GridSettings& settings, Amplitudes& amps) override final {
            
            
            std::vector<float> speakerAmps(speakerPositions2D.size(), 0.);
            
            
            float phantomAmp = 0.;
            
            for(int i = 0; i < triangles.size(); i++){

                float triangleAmps[3] = {0., 0., 0.};
                triangles[i].getAmps(source, triangleAmps);
                
                // first amp: first speaker
                speakerAmps[i] = std::max<float>(speakerAmps[i], triangleAmps[0]);
                // second amp: first speaker
                speakerAmps[(i+1)%speakerPositions2D.size()] = std::max<float>(speakerAmps[(i+1)%speakerPositions2D.size()], triangleAmps[1]);
                // third amp: phantom speaker
                phantomAmp = std::max<float>(phantomAmp, triangleAmps[2]);
            }
            
            // increment speaker amps with phantom amp (but clip at 1)
            float phantomValue = phantomAmp / static_cast<float>(speakerAmps.size());
            for(int i = 0; i < speakerAmps.size(); i++){
                speakerAmps[i] += phantomValue;
                if(speakerAmps[i] > 1.)
                    speakerAmps[i] = 1.;
            }

            
            // set amps
            for(int i = 0; i < speakers.size(); i++)
                amps.set(speakers[i]->channel, speakerAmps[i]);

        }

        
        static bool isValid(const std::vector<Speaker*>& speakers){
            return true;
        }
        
    private:
        
        class Triangle
        {
        public:
            Triangle(const std::vector<glm::vec2>& speakerPositions) : speakerPositions(speakerPositions)
            {
                area = triangleArea(speakerPositions[0], speakerPositions[1], speakerPositions[2]);
            }
            
            /**
             * Algorithm copied from TriangleShape. Sets amp1, amp2, amp3 given the source.
             */
            void getAmps(const Source& source, float amps[3])
            {
                for(int i = 0; i < 3; i++){
                    
                    glm::vec2 closestPoint = source.getClosestPointTo2D(speakerPositions[i]);
                    
                    if(pointInPoly(closestPoint, speakerPositions)){
                        
                        float areaRatio = triangleArea(closestPoint, speakerPositions[(i+1)%3], speakerPositions[(i+2)%3]) / area;
                        
                        // clip (can be a bit higher than 1 because of epsilon errors)
                        if(areaRatio > 1.)
                            areaRatio = 1.;
                        else if(areaRatio < 0.)
                            areaRatio = 0.;
                        
                        amps[i] = areaRatio;
                        
                    }
                }
            }
            
        private:
            std::vector<glm::vec2> speakerPositions;
            float area;
            
        };
        
        
        std::vector<glm::vec2> speakerPositions2D;
        glm::vec2 center;
        std::vector<Triangle> triangles;
        
    };
    
    
}


#endif /* CircleShape_hpp */
