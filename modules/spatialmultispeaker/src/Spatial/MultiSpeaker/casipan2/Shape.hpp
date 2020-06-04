//
//  Shape.hpp
//  4dengine
//
//  Created by Casimir Geelhoed on 11/07/2018.
//
//

#ifndef Shape_hpp
#define Shape_hpp

#include <vector>
#include <memory>
#include <math.h>
#include <map>

#include "Speaker.hpp"
#include "Source.hpp"
#include "Amplitudes.hpp"

namespace casipan {

    class Shape {
        
    public:
        
        Shape(const std::vector<Speaker*>& speakers) : speakers(speakers) { }
        virtual ~Shape() {};

        virtual void getAllAmplitudesForSource(const Source& source, const GridSettings& settings, Amplitudes& amps) = 0;
        
        
    protected:
        std::vector<Speaker*> speakers;
        
    };

}


#endif /* Shape_hpp */
