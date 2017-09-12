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
        
        
        class NAPAPI StereoPannerComponent : public AudioComponent {
            RTTI_ENABLE(nap::audio::AudioComponent)
        public:
            StereoPannerComponent() : AudioComponent() { }
            
            // Type to instantiate
            const rtti::TypeInfo getInstanceType() const override
            {
                return RTTI_OF(StereoPannerComponentInstance);
            }
            
        public:
            ComponentPtr mInput;
            float mPanning = 0.5;
            
        };

        
        class NAPAPI StereoPannerComponentInstance : public AudioComponentInstance {
            RTTI_ENABLE(nap::audio::AudioComponentInstance)
        public:
            StereoPannerComponentInstance(EntityInstance& entity, Component& resource) : AudioComponentInstance(entity, resource) { }
            
            // Initialize the component
            bool init(EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState) override;
            
            OutputPin* getOutputForChannel(int channel) override;
            int getChannelCount() const override { return 2; }
            
            
        private:
            std::unique_ptr<StereoPanner> stereoPanner = nullptr;
        };

    }
        
}
