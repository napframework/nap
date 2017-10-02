#include "audiocomponent.h"


// Nap includes
#include <nap/entity.h>
#include <nap/core.h>

// Audio includes
#include <service/audioservice.h>


// RTTI
RTTI_DEFINE_BASE(nap::audio::AudioComponent)
RTTI_DEFINE_BASE(nap::audio::AudioComponentInstance)


namespace nap {
    
    namespace audio {
    
        NodeManager& AudioComponentInstance::getNodeManager()
        {
            return getEntityInstance()->getCore()->getService<AudioService>()->getNodeManager();
        }
        
    }
    
}
