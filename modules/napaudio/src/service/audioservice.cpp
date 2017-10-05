#include "audioservice.h"
#include <core/graph.h>
#include <core/voice.h>

RTTI_DEFINE_BASE(nap::audio::AudioService)

namespace nap {
    
    namespace audio {
        
        void AudioService::registerObjectCreators(rtti::Factory& factory)
        {
            factory.addObjectCreator(std::make_unique<GraphObjectCreator>(getNodeManager()));
            factory.addObjectCreator(std::make_unique<VoiceObjectCreator>(getNodeManager()));
        }
        
    }
    
}
