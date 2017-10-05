#pragma once

// Nap includes
#include <nap/component.h>
#include <nap/componentptr.h>

// Audio includes
#include <core/audionode.h>
#include <core/audiocomponent.h>

namespace nap {
    
    namespace audio {
    
        class OutputComponentInstance;
        
        
        /**
         * Component that routs output from another audio component to the audio interface.
         */
        class NAPAPI OutputComponent : public Component {
            RTTI_ENABLE(nap::Component)
            DECLARE_COMPONENT(OutputComponent, OutputComponentInstance)
            
        public:
            OutputComponent() : nap::Component() { }
            
        public:
            // Properties
            nap::ComponentPtr<AudioComponent> mInput; /**<  The component whose audio output to rout to the interface */
            
            /**
             * The size of this vector indicates the number of channels this component outputs.
             * Each element of the vector indicates which channel from the input will be routed on the corresponding output channel.  
             */
            std::vector<int> mChannelRouting = { 0 };
        };

        
        /**
         * Instance of component that routs output from another audio component to the audio interface
         */
        class NAPAPI OutputComponentInstance : public ComponentInstance {
            RTTI_ENABLE(nap::ComponentInstance)
        public:
            OutputComponentInstance(EntityInstance& entity, Component& resource) : nap::ComponentInstance(entity, resource) { }
            
            // Initialize the component
            bool init(EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState) override;
            
        private:
            std::vector<std::unique_ptr<OutputNode>> mOutputs;
        };

    }
    
}
