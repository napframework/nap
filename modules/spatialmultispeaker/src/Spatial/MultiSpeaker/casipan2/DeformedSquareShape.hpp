//
//  DeformedSquareShape.hpp
//  Project
//
//  Created by Casimir Geelhoed on 28/10/2019.
//
//

#ifndef DeformedSquareShape_hpp
#define DeformedSquareShape_hpp

#include <stdio.h>
#include <iostream>

#include "Shape.hpp"
#include "Functions.hpp"


namespace casipan {
    
    class DeformedSquareShape : public Shape {
        
    public:
        
        DeformedSquareShape(const std::vector<Speaker*>& speakers) : Shape(speakers) {
            
            speakerPositions2D.push_back(to2D(speakers[0]->position));
            speakerPositions2D.push_back(to2D(speakers[1]->position));
            speakerPositions2D.push_back(to2D(speakers[3]->position));
            speakerPositions2D.push_back(to2D(speakers[2]->position));
            
        }
        
        void getAllAmplitudesForSource(const Source& source, const GridSettings& settings, Amplitudes& amps) override final {
            
            for(int i = 0; i < 4; i++){
                
                const glm::vec2 closestPointToSpeaker = source.getClosestPointTo2D(to2D(speakers[i]->position));
                
                if(isInside(closestPointToSpeaker)){
                    
                    const glm::vec2 relativePosition = mapQuadrilateralPointToRectanglePoint(closestPointToSpeaker, speakerPositions2D[0], speakerPositions2D[1], speakerPositions2D[2], speakerPositions2D[3]);
                    
                    const float valX = clamp<float>(relativePosition.x, 0., 1.);
                    const float valZ = clamp<float>(relativePosition.y, 0., 1.);

                    if(i == 0)
                        amps.set(speakers[0]->channel, (1. - valX)  * (1. - valZ));
                    else if(i == 1)
                        amps.set(speakers[1]->channel, valX  * (1. - valZ));
                    else if(i == 2)
                        amps.set(speakers[2]->channel, (1. - valX) * valZ);
                    else if(i == 3)
                        amps.set(speakers[3]->channel, valX * valZ);
                                    
                }
                
            }
            
        }
        
        static bool isValid(const std::vector<Speaker*>& speakers){
            if(speakers.size() != 4)
                return false;
            return true;
        }
        
    private:
        inline bool isInside(const glm::vec2& position) const {
            return pointInPoly(position, speakerPositions2D);
        }
        
        std::vector<glm::vec2> speakerPositions2D;
        
    };
    
}

#endif /* DeformedSquareShape_hpp */
