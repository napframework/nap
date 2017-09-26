#include "oscillatorcomponent.h"

// Nap includes
#include <nap/entity.h>

// RTTI
RTTI_BEGIN_CLASS(nap::audio::OscillatorComponent)
    RTTI_PROPERTY("ChannelCount", &nap::audio::OscillatorComponent::mChannelCount, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("Amplitude", &nap::audio::OscillatorComponent::mAmplitude, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("Frequency", &nap::audio::OscillatorComponent::mFrequency, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::OscillatorComponentInstance)
    RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

namespace nap
{
    
    namespace audio
    {
    
        bool OscillatorComponentInstance::init(EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState)
        {
            OscillatorComponent* resource = rtti_cast<OscillatorComponent>(getComponent());
            
            AudioComponentInstance* input = resource->mFmInput.get();
            auto& nodeManager = getNodeManager();
            
            for (auto channel = 0; channel < resource->mChannelCount; ++channel)
            {
                if (resource->mFrequency.empty())
                {
                    errorState.fail("%s Oscillator frequency is empty array", resource->mID.c_str());
                    return false;
                }
                
                if (resource->mAmplitude.empty())
                {
                    errorState.fail("%s Oscillator amplitude is empty array", resource->mID.c_str());
                    return false;
                }
                
                mOscillators.emplace_back(std::make_unique<Oscillator>(nodeManager, mWave));
                mOscillators[channel]->setFrequency(resource->mFrequency[channel % resource->mFrequency.size()]);
                mOscillators[channel]->setAmplitude(resource->mAmplitude[channel % resource->mAmplitude.size()]);
            }
            
            return true;
        }
        
    }
    
}
