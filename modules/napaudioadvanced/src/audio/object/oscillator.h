#pragma once

// Nap includes
#include <nap/resourceptr.h>

// Audio includes
#include <audio/core/audioobject.h>
#include <audio/node/oscillatornode.h>
#include <audio/utility/safeptr.h>

namespace nap
{
    
    namespace audio
    {
        
        /**
         * Multichannel oscillator object.
         */
        class Oscillator : public MultiChannelObject
        {
            RTTI_ENABLE(MultiChannelObject)
            
        public:
            int mChannelCount = 1; ///< property: 'ChannelCount' indicates number of channels, each channel holds an oscillator node.
            std::vector<ControllerValue> mFrequency = { 220.f }; ///< property: 'Frequency' array of frequency values that will be mapped on the oscillators on each channel
            std::vector<ControllerValue> mAmplitude = { 1.f }; ///< property: 'Amplitude' array of amplitude values that will be mapped on the oscillators on each channel
            ResourcePtr<AudioObject> mFmInput; ///< property: 'FmInput' audio object of which the outputs will modulate the frequencies of the oscillators on each channel.
            
        private:
            SafeOwner<Node> createNode(int channel, AudioService& audioService) override
            {
                if (mWaveTable == nullptr)
                    mWaveTable = audioService.makeSafe<WaveTable>(2048);
                SafeOwner<OscillatorNode> node = audioService.makeSafe<OscillatorNode>(audioService.getNodeManager(), mWaveTable.get());
                node->setFrequency(mFrequency[channel % mFrequency.size()]);
                node->setAmplitude(mAmplitude[channel % mAmplitude.size()]);
                if (mFmInput != nullptr)
                {
                    node->fmInput.connect(mFmInput->getInstance()->getOutputForChannel(channel % mFmInput->getInstance()->getChannelCount()));
                }
                
                return std::move(node);
            }
            
            int getChannelCount() const override { return mChannelCount; }
            SafeOwner<WaveTable> mWaveTable = nullptr;
        };
        
       
    }
    
}
