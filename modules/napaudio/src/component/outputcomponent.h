#pragma once

// Nap includes
#include <nap/component.h>
#include <nap/componentptr.h>

// Audio includes
#include <node/audionode.h>

namespace nap {
    
    namespace audio {
    
        class OutputComponentInstance;
        
        
        /**
         * Component that routs output from another audio component to the audio interface.
         */
        class NAPAPI OutputComponent : public Component {
            RTTI_ENABLE(nap::Component)
        public:
            OutputComponent() : nap::Component() { }
            
            // Type to instantiate
            const rtti::TypeInfo getInstanceType() const override
            {
                return RTTI_OF(OutputComponentInstance);
            }
            
        public:
            // Properties
            nap::ComponentPtr mInput; /**<  The component whose audio output to rout to the interface */
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
