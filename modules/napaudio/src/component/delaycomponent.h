#pragma once

// Nap includes
#include <nap/component.h>

// Audio includes
#include <component/audiocomponent.h>
#include <node/delaynode.h>

namespace nap {
    
    namespace audio {
    
        class DelayComponentInstance;
        
        
        class NAPAPI DelayComponent : public AudioComponent {
            RTTI_ENABLE(nap::audio::AudioComponent)
        public:
            DelayComponent() : AudioComponent() { }
            
            // Type to instantiate
            const rtti::TypeInfo getInstanceType() const override
            {
                return RTTI_OF(DelayComponentInstance);
            }

        public:
            // Properties
            
            ComponentPtr mInput; /**< The audio component to get the input signal to be panned from */
            std::vector<TimeValue> mTime = { 500 };
            ControllerValue mDryWet = 0.5;
            ControllerValue mFeedback = 0.2;
            std::vector<int> mInputRouting = { 0 };
            
        };

        
        class NAPAPI DelayComponentInstance : public AudioComponentInstance {
            RTTI_ENABLE(nap::audio::AudioComponentInstance)
        public:
            DelayComponentInstance(EntityInstance& entity, Component& resource) : AudioComponentInstance(entity, resource) { }
            
            // Initialize the component
            bool init(EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState) override;
            
            OutputPin& getOutputForChannel(int channel) override final { return mDelays[channel]->output; }
            int getChannelCount() const override final { return mDelays.size(); }
            
        private:
            std::vector<std::unique_ptr<DelayNode>> mDelays;
        };

    }
    
}
