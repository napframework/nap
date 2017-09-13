#pragma once

// Nap includes
#include <nap/component.h>
#include <nap/componentptr.h>

// Audio includes
#include <node/audionode.h>

namespace nap {
    
    namespace audio {
    
        class AudioComponentInstance;
        
        
        /**
         * Component that generates audio output for one or more channels
         */
        class NAPAPI AudioComponent : public Component {
            RTTI_ENABLE(nap::Component)
        public:
            AudioComponent() : nap::Component() { }
            
            // Type to instantiate
            const rtti::TypeInfo getInstanceType() const override
            {
                return RTTI_OF(AudioComponentInstance);
            }
            
        private:
        };

        
        /**
         * Instance of a component that generates audio output for one or more channels
         */
        class NAPAPI AudioComponentInstance : public ComponentInstance {
            RTTI_ENABLE(nap::ComponentInstance)
        public:
            AudioComponentInstance(EntityInstance& entity, Component& resource) : nap::ComponentInstance(entity, resource) { }
            
            /**
             * Has to be overridden to specify an output pin that contains this components output.
             * Does not need to do error checking, the caller has to make sure @channel < @getChannelCount().
             * @param channel: the channel that the output pin needs to be returned for
             * @return: an output pin on a node that is owned by this component instance
             */
            virtual OutputPin& getOutputForChannel(int channel) = 0;
            
            /**
             * @return: the number of audio channels that this copmonent outputs
             */
            virtual int getChannelCount() const = 0;
            
        protected:
            NodeManager& getNodeManager();
            
        };

    }
        
}
