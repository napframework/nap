//
//  TriangleShape.hpp
//  testframework
//
//  Created by Casimir Geelhoed on 17/08/2018.
//
//

#ifndef TriangleShape_hpp
#define TriangleShape_hpp


#include "Shape.hpp"
#include "Functions.hpp"

namespace casipan {

    class TriangleShape : public Shape {
        
    public:
        
        TriangleShape(const std::vector<Speaker*> speakers) : Shape(speakers) {
            
            for(int i = 0; i < 3; i++)
                speakerPositions2D.push_back(to2D(speakers[i]->position));
            
            totalArea = triangleArea(speakerPositions2D[0], speakerPositions2D[1], speakerPositions2D[2]);
        }
        
        
        void getAllAmplitudesForSource(const Source& source, const GridSettings& settings, Amplitudes& amps) override final {

            for(int i = 0; i < 3; i++){
            
                glm::vec2 closestPoint = source.getClosestPointTo2D(speakerPositions2D[i]);
                
                if(pointInPoly(closestPoint, speakerPositions2D)){
                    
                    float areaRatio = triangleArea(closestPoint, speakerPositions2D[(i+1)%3],speakerPositions2D[(i+2)%3]) / totalArea;
                    
                    // clip (can be a bit higher than 1 because of epsilon errors)
                    if(areaRatio > 1.)
                        areaRatio = 1.;
                    else if(areaRatio < 0.)
                        areaRatio = 0.;
                    
                    amps.set(speakers[i]->channel, areaRatio);
                    
                }
            }
            
        }

        static bool isValid(const std::vector<Speaker*>& speakers){
            return speakers.size() == 3;
        }
        
    private:

        float totalArea;
        std::vector<glm::vec2> speakerPositions2D;
        
    };
    
}


#endif /* TriangleShape_hpp */
