#pragma once

// Nap includes
#include <nap/component.h>
#include <nap/componentptr.h>

// Audio include
#include <component/audiocomponent.h>
#include <node/stereopanner.h>

namespace nap {
        
    namespace audio {
    
        class StereoPannerComponentInstance;
        
        
        /**
         * Component to pan a mono or stereo audio signal across two channels
         */
        class NAPAPI StereoPannerComponent : public AudioComponent {
            RTTI_ENABLE(nap::audio::AudioComponent)
            DECLARE_COMPONENT(StereoPannerComponent, StereoPannerComponentInstance)
            
        public:
            StereoPannerComponent() : AudioComponent() { }
            
        public:
            ComponentPtr<AudioComponent> mInput; /**< The audio component to get the input signal to be panned from */
            float mPanning = 0.5; /**< The panning value: 0 = far left, 1 = far right */
            
        };

        
        /**
         * Instance of component to pan a mono or stereo audio signal across two channels
         */
        class NAPAPI StereoPannerComponentInstance : public AudioComponentInstance {
            RTTI_ENABLE(nap::audio::AudioComponentInstance)
        public:
            StereoPannerComponentInstance(EntityInstance& entity, Component& resource) : AudioComponentInstance(entity, resource) { }
            
            // Initialize the component
            bool init(EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState) override;
            
            OutputPin& getOutputForChannel(int channel) override final;
            int getChannelCount() const override final { return 2; }
            
            
        private:
            std::unique_ptr<StereoPanner> stereoPanner = nullptr;
        };

    }
        
}
