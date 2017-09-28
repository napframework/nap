#pragma once

// Nap includes
#include <nap/component.h>

// Audio includes
#include <component/audiocomponent.h>
#include <node/oscillatornode.h>

namespace nap
{
    
    namespace audio
    {
    
        class OscillatorComponentInstance;
        
        
        class NAPAPI OscillatorComponent : public AudioComponent
        {
            RTTI_ENABLE(AudioComponent)
            DECLARE_COMPONENT(OscillatorComponent, OscillatorComponentInstance)
            
        public:
            OscillatorComponent() : AudioComponent() { }
            
            ComponentPtr<AudioComponent> mFmInput = nullptr; /**< The audio component to get the FM input signal from */
            
            std::vector<ControllerValue> mFrequency = { 220.f };
            std::vector<ControllerValue> mAmplitude = { 1.f };
            int mChannelCount = 1;

        private:
        };

        
        class NAPAPI OscillatorComponentInstance : public AudioComponentInstance
        {
            RTTI_ENABLE(AudioComponentInstance)
            
        public:
            OscillatorComponentInstance(EntityInstance& entity, Component& resource) : AudioComponentInstance(entity, resource) { }
            
            // Initialize the component
            bool init(EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState) override;
            
        private:
            OutputPin& getOutputForChannel(int channel) override final { return mOscillators[channel]->output; }
            int getChannelCount() const override final { return mOscillators.size(); }
            
            std::vector<std::unique_ptr<OscillatorNode>> mOscillators;
            WaveTable mWave = { 2048 };
        };
        
    }
    
}
