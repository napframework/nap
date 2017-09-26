#pragma once

#include <rtti/rttiobject.h>

// Nap includes
#include <nap/component.h>

// Audio includes
#include <component/audiocomponent.h>
#include <node/envelopenode.h>

namespace nap
{
    
    namespace audio
    {
        
        // Forward declares
        class EnvelopeComponentInstance;
        
        
        class NAPAPI EnvelopeComponent : public AudioComponent
        {
            RTTI_ENABLE(AudioComponent)
            DECLARE_COMPONENT(EnvelopeComponent, EnvelopeComponentInstance)
            
        public:
            EnvelopeComponent() : AudioComponent() { }
            
            std::vector<EnvelopeGenerator::Segment> mSegments;
        };

        
        class NAPAPI EnvelopeComponentInstance : public AudioComponentInstance
        {
            RTTI_ENABLE(AudioComponentInstance)
        public:
            EnvelopeComponentInstance(EntityInstance& entity, Component& resource) : AudioComponentInstance(entity, resource) { }
            
            // Initialize the component
            bool init(EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState) override;
            
        private:
            OutputPin& getOutputForChannel(int channel) override final { return mEnvelopeGenerator->output; }
            int getChannelCount() const override final { return 1; }
            
            std::unique_ptr<EnvelopeGenerator> mEnvelopeGenerator = nullptr;
        };
        
    }
    
}
