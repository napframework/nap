//
//  StaticShape.hpp
//  4dengine
//
//  Created by Casimir Geelhoed on 19/07/2018.
//
//

#ifndef StaticShape_hpp
#define StaticShape_hpp

#include <stdio.h>
#include <vector>
#include "Shape.hpp"

#include <glm/glm.hpp>


namespace casipan {

    using glm::vec3;

    // shape that always returns 1 for all speakers in shape.
    class StaticShape : public Shape {
        
    public:
        
        StaticShape(const std::vector<Speaker*> speakers) : Shape(speakers) {
            
        }
        
        
        void getAllAmplitudesForSource(const Source& source, const GridSettings& settings, Amplitudes& amps) override final {
            
            for(int i = 0; i < speakers.size(); i++)
                amps.set(speakers[i]->channel, 1.);
                
        }
        
        static bool isValid(const std::vector<Speaker*>& speakers){
            return true;
        }
        
        
    };

}

#endif /* StaticShape_hpp */
