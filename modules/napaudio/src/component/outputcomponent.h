#pragma once

// Nap includes
#include <nap/component.h>
#include <nap/componentptr.h>

// Audio includes
#include <node/audionode.h>
#include <device/audiointerface.h>

namespace nap {
    
    namespace audio {
    
        class OutputComponentInstance;
        
        
        class NAPAPI OutputComponent : public Component {
            RTTI_ENABLE(nap::Component)
        public:
            OutputComponent() : nap::Component() { }
            
            // Type to instantiate
            const rtti::TypeInfo getInstanceType() const override
            {
                return RTTI_OF(OutputComponentInstance);
            }
            
            /**
             * Pointer to the audio interface the component runs on
             */
            ObjectPtr<AudioInterface> mAudioInterface;
            
        public:
            // Properties
            nap::ComponentPtr mInput;
        };

        
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
