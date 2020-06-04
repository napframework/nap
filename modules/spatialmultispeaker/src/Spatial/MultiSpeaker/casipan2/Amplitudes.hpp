//
//  Amplitudes.hpp
//  4dengine
//
//  Created by Casimir Geelhoed on 25/07/2018.
//
//

#ifndef Amplitudes_hpp
#define Amplitudes_hpp

#include <stdio.h>
#include <vector>
#include <algorithm>

namespace casipan {

    static const int MAX_CHANNELS = 256;

    /* Datastructure of amplitudes per channel. */

    // TODO c array instead of vector

    class Amplitudes {
        
    public:
        
        Amplitudes(int size){
            amps.resize(size, 0.f);
        }
        
        std::vector<float>& getAmps(){
            return amps;
        }
        
        void set(int index, float value, bool alwaysOverride = false){
            if(alwaysOverride)
                amps[index] = value;
            else
                amps[index] = std::max<float>(value, amps[index]);
        }
        
        /** 
         * Adds a value to an amplitude. Can be used to distribute the value of a phantom speaker.
         */
        void add(int index, float value, bool limit = true)
        {
            if(limit)
                amps[index] = std::min<float>(1.f, amps[index] + value);
            else
                amps[index] = amps[index] + value;
        }
        
    private:
        
        std::vector<float> amps;
        
    };

}

#endif /* Amplitudes_hpp */
