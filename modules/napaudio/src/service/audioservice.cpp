#include "audioservice.h"
#include <object/audioobject.h>
#include <object/objects.h>

namespace nap {
    
    namespace audio {
        
        void AudioService::registerObjectCreators(rtti::Factory& factory)
        {
            factory.addObjectCreator(std::make_unique<AudioObjectCreator>(getNodeManager()));
//            factory.addObjectCreator(std::make_unique<OscillatorCreator>(getNodeManager()));
        }
        
    }
    
}
