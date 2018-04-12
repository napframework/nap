#include "inputcomponent.h"

// Nap includes
#include <entity.h>
#include <nap/core.h>

// Audio includes
#include <audio/core/audionodemanager.h>
#include <audio/service/audioservice.h>

// RTTI
RTTI_BEGIN_CLASS(nap::audio::AudioInputComponent)
    RTTI_PROPERTY("Channels", &nap::audio::AudioInputComponent::mChannels, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("Gain", &nap::audio::AudioInputComponent::mChannels, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::AudioInputComponentInstance)
    RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

namespace nap
{
    
    namespace audio
    {
    
        bool AudioInputComponentInstance::init(utility::ErrorState& errorState)
        {
            auto resource = getComponent<AudioInputComponent>();
            auto nodeManager = &getNodeManager();
            auto audioService = &getAudioService();
            
            mGain = resource->mGain;

            mGainControl = audioService->makeSafe<ControlNode>(*nodeManager);
            mGainControl->setValue(mGain);

            for (auto channel = 0; channel < resource->mChannels.size(); ++channel)
            {
                if (resource->mChannels[channel] >= nodeManager->getInputChannelCount())
                {
                    errorState.fail("AudioInputComponent: Input channel out of bounds");
                    return false;
                }
                
                auto inputNode = audioService->makeSafe<InputNode>(*nodeManager);
                inputNode->setInputChannel(resource->mChannels[channel]);
                
                auto gainNode = audioService->makeSafe<GainNode>(*nodeManager);
                gainNode->inputs.connect(inputNode->audioOutput);
                gainNode->inputs.connect(mGainControl->output);
                
                mInputNodes.emplace_back(std::move(inputNode));
                mGainNodes.emplace_back(std::move(gainNode));
            }
            return true;
        }
        
        
        void AudioInputComponentInstance::setGain(ControllerValue gain)
        {
            getAudioService().enqueueTask([&, gain](){ mGainControl->ramp(gain, 1); });
        }
        
        
        ControllerValue AudioInputComponentInstance::getGain() const
        {
            return mGain;
        }

        
    }
    
}
