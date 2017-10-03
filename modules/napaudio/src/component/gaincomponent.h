#pragma once

// Nap includes
#include <nap/component.h>
#include <nap/componentptr.h>

// Audio includes
#include <component/audiocomponent.h>
#include <node/gainnode.h>

namespace nap
{
    
    namespace audio
    {
    
        class GainComponentInstance;
        
        
        class NAPAPI GainComponent : public AudioComponent
        {
            RTTI_ENABLE(AudioComponent)
            DECLARE_COMPONENT(GainComponent, GainComponentInstance)
            
        public:
            GainComponent() : AudioComponent() { }

            std::vector<ComponentPtr<AudioComponent>> mInputs;
            std::vector<ControllerValue> mGain = { 1 };
        };

        
        class NAPAPI GainComponentInstance : public AudioComponentInstance
        {
            RTTI_ENABLE(AudioComponentInstance)
        public:
            GainComponentInstance(EntityInstance& entity, Component& resource) : AudioComponentInstance(entity, resource) { }
            
            // Initialize the component
            bool init(EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState) override;
            
        private:
            OutputPin& getOutputForChannel(int channel) override final { return mGains[channel]->audioOutput; }
            int getChannelCount() const override final { return mGains.size(); }
            
            std::vector<std::unique_ptr<GainNode>> mGains;
        };
        
    }
    
}
