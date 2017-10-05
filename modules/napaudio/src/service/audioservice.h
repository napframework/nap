#pragma once

// Nap includes
#include <nap/service.h>

// Audio includes
#include <core/audionodemanager.h>

namespace nap {
    
    namespace audio  {
        
        
        /**
         * To support different types of audio services this base class defines the interface that all audio services have to implement. 
         * Examples of different audio services are: 
         * - the AudioDeviceService that communicates with a hardware audio device
         * - An AudioPluginService that manages input and output of an audio plugin like VST or AudioUnit
         * Every audio service has to expose a node manager to the outside world that components can add nodes to to perform the audio processing.
         */
        class NAPAPI AudioService : public Service {
            RTTI_ENABLE(nap::Service)
            
        public:
            AudioService() : Service() { }
            
            /**
             * Register specific object creators
             */
            void registerObjectCreators(rtti::Factory& factory) override;
            
            virtual NodeManager& getNodeManager() = 0;
        };
        
    }
    
}
