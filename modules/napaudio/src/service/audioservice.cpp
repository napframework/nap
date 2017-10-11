#include "audioservice.h"

// Audio includes
#include <core/graph.h>
#include <core/voice.h>

// Third party includes
#include <mpg123.h>

RTTI_DEFINE_BASE(nap::audio::AudioService)

namespace nap {
    
    namespace audio {
        
        AudioService::AudioService() : Service()
        {
            // Initialize mpg123 library
            mpg123_init();
        }
        
        
        AudioService::~AudioService()
        {
            // Uninitialize mpg123 library
            mpg123_exit();
        }

        
        void AudioService::registerObjectCreators(rtti::Factory& factory)
        {
            factory.addObjectCreator(std::make_unique<GraphObjectCreator>(getNodeManager()));
            factory.addObjectCreator(std::make_unique<VoiceObjectCreator>(getNodeManager()));
        }
        
    }
    
}
