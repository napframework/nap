#include "audioservice.h"
#include <object/audiograph.h>

namespace nap {
    
    namespace audio {
        
        void AudioService::registerObjectCreators(rtti::Factory& factory)
        {
            factory.addObjectCreator(std::make_unique<GraphObjectCreator>(getNodeManager()));
        }
        
    }
    
}
