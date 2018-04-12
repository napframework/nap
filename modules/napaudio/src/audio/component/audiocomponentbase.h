#pragma once

// Nap includes
#include <component.h>
#include <componentptr.h>
#include <rtti/objectptr.h>

// Audio includes
#include <audio/core/audionode.h>

namespace nap
{
    
    namespace audio
    {
    
        class AudioService;
        class AudioComponentBaseInstance;
        
        
        /**
         * Component that generates audio output for one or more channels.
         */
        class NAPAPI AudioComponentBase : public Component
        {
            RTTI_ENABLE(nap::Component)
            DECLARE_COMPONENT(AudioComponent, AudioComponentBaseInstance)
            
        public:
            AudioComponentBase() : nap::Component() { }

        private:
        };

        
        /**
         * Instance of a component that generates audio output for one or more channels.
         */
        class NAPAPI AudioComponentBaseInstance : public ComponentInstance
        {
            RTTI_ENABLE(nap::ComponentInstance)
            
        public:
            AudioComponentBaseInstance(EntityInstance& entity, Component& resource) : nap::ComponentInstance(entity, resource) { }
            
            /**
             * Override this method to specify the number of audio channels output by this component.
             */
            virtual int getChannelCount() const = 0;
            
            /**
             * Override this to return the output pin that outputs audio data for the specified channel.
             */
            virtual OutputPin& getOutputForChannel(int channel) = 0;
            
        protected:
            /**
             * Returns the node system's node manager that the audio runs on
             */
            NodeManager& getNodeManager();
            
            /**
             * Returns the audio service
             */
            AudioService& getAudioService();
        };

    }
        
}
