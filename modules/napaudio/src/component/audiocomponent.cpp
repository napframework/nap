#include "audiocomponent.h"


// Nap includes
#include <nap/entity.h>
#include <nap/core.h>

// Audio includes
#include <service/audioservice.h>


// RTTI
RTTI_BEGIN_CLASS(nap::audio::AudioComponent)
RTTI_END_CLASS


namespace nap {
    
    namespace audio {
    
        NodeManager& AudioComponentInstance::getNodeManager()
        {
            return getEntityInstance()->getCore()->getService<AudioService>()->getNodeManager();
        }
        
    }
    
}
