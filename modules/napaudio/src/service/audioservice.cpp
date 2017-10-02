#include "audioservice.h"
#include <graph/audiograph.h>
#include <graph/voicegraph.h>

RTTI_DEFINE_BASE(nap::audio::AudioService)

namespace nap {
    
    namespace audio {
        
        void AudioService::registerObjectCreators(rtti::Factory& factory)
        {
            factory.addObjectCreator(std::make_unique<GraphObjectCreator>(getNodeManager()));
            factory.addObjectCreator(std::make_unique<VoiceGraphObjectCreator>(getNodeManager()));
        }
        
    }
    
}
