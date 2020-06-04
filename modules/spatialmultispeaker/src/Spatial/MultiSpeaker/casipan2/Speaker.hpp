//
//  Speaker.hpp
//  4dengine
//
//  Created by Casimir Geelhoed on 11/07/2018.
//
//

#ifndef Speaker_hpp
#define Speaker_hpp

#include <stdio.h>
#include <string>
#include <glm/vec3.hpp>

namespace casipan {

    struct Speaker {
        
        Speaker(int channel, std::string name, std::string speakerType, glm::vec3 position)
        : channel(channel), name(name), speakerType(speakerType), position(position) {}
        
        int channel; // output channel
        std::string name; // display name
        std::string speakerType; // speakertype as string. not used in 4dpan (yet?). Interpreted by Panner to load in appropriate masterchain.
        glm::vec3 position; // 3D position
        
    };

}


#endif /* Speaker_hpp */
