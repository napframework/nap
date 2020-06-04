#include "DryWetEnableEffectProcessor.h"

// Spatial Audio includes
#include <Spatial/Audio/FastDelay.h>

// audio includes
#include <audio/object/gain.h>
#include <audio/core/audioobject.h>

RTTI_DEFINE_BASE(nap::spatial::DryWetEnableEffectProcessor)


namespace nap
{
    
    namespace spatial
    {
        
        
        bool DryWetEnableEffectProcessor::init(audio::AudioService& audioService, int channelCount, utility::ErrorState& errorState)
        {
            mChannelCount = channelCount;
            
            // init custom DSP
            mDSP = createDSP(audioService, channelCount, errorState);
            if (mDSP == nullptr)
                return false;
            if (mDSP->getChannelCount() != channelCount)
            {
                errorState.fail("Custom DSP in DryWetEnableEffectProcessor has inappropriate channel count");
                return false;
            }

            mDryWetInstance = std::make_unique<audio::ParallelNodeObjectInstance<audio::DryWetEnableNode>>();
            if (!mDryWetInstance->init(channelCount, audioService.getNodeManager(), errorState))
            {
                errorState.fail("Failed to initialize DryWetEnable for DryWetEnableEffectProcessor");
                return false;
            }

            for (auto channel = 0; channel < mDryWetInstance->getChannelCount(); ++channel)
            {
                auto dryWetNode = mDryWetInstance->getChannel(channel);
                dryWetNode->wetInput.connect(*mDSP->getOutputForChannel(channel));
                // set dry wet by default to 1.0
                dryWetNode->setDryWet(1.0);
            }
            
            return true;
        }
        
        
        void DryWetEnableEffectProcessor::connect(unsigned int channel, audio::OutputPin& pin)
        {
            mDryWetInstance->getChannel(channel)->dryInput.connect(pin);
            connectToDSP(channel, pin);
        }
        
        
        void DryWetEnableEffectProcessor::setEnable(bool value)
        {
            for (auto channel = 0; channel < mDryWetInstance->getChannelCount(); ++channel)
                mDryWetInstance->getChannel(channel)->setEnabled(value);
        }
        
        void DryWetEnableEffectProcessor::setDryWet(float value)
        {
            for (auto channel = 0; channel < mDryWetInstance->getChannelCount(); ++channel)
                mDryWetInstance->getChannel(channel)->setDryWet(value);
        }
        
        void DryWetEnableEffectProcessor::setDry(float value)
        {
            for (auto channel = 0; channel < mDryWetInstance->getChannelCount(); ++channel)
                mDryWetInstance->getChannel(channel)->setDry(value);
        }
        
        void DryWetEnableEffectProcessor::setWet(float value)
        {
            for (auto channel = 0; channel < mDryWetInstance->getChannelCount(); ++channel)
                mDryWetInstance->getChannel(channel)->setWet(value);
        }
        
        
        void DryWetEnableEffectProcessor::setEnable(bool value, int channel)
        {
            mDryWetInstance->getChannel(channel)->setEnabled(value);
        }
        
        void DryWetEnableEffectProcessor::setDryWet(float value, int channel)
        {
            mDryWetInstance->getChannel(channel)->setDryWet(value);
        }
        
        void DryWetEnableEffectProcessor::setDry(float value, int channel)
        {
            mDryWetInstance->getChannel(channel)->setDry(value);
        }
        
        void DryWetEnableEffectProcessor::setWet(float value, int channel)
        {
            mDryWetInstance->getChannel(channel)->setWet(value);
        }

        
    }
}
