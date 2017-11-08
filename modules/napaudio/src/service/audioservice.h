#pragma once

// Nap includes
#include <nap/service.h>

// Audio includes
#include <core/audionodemanager.h>

namespace nap {
    
    namespace audio  {
        
        
        /**
         * This base class defines the interface that all audio services have to implement. This way different types of audio services can be implemented that work with different backends but provide the same interface.
         * Examples of different types of audio services are:
         * - the AudioDeviceService that communicates with a hardware audio device
         * - An AudioPluginService that manages input and output of an audio plugin like VST or AudioUnit
         * Every audio service has to expose a node manager to the outside world that audio components can add nodes to in order to perform the audio processing.
         */
        class NAPAPI AudioService : public Service {
            RTTI_ENABLE(nap::Service)
            
        public:
            AudioService();
            virtual ~AudioService();
            
            /**
             * Register specific object creators
             */
            void registerObjectCreators(rtti::Factory& factory) override;
            
            /**
             * This method has to be overwritten by descendants to provede a node manager owned by the service.
             */
            virtual NodeManager& getNodeManager() = 0;
        };
        
    }
    
}
