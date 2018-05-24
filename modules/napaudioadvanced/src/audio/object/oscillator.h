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
        
        class Oscillator : public MultiChannelObject
        {
            RTTI_ENABLE(MultiChannelObject)
            
        public:
            int mChannelCount = 1;
            std::vector<ControllerValue> mFrequency = { 220.f };
            std::vector<ControllerValue> mAmplitude = { 1.f };
            ResourcePtr<AudioObject> mFmInput;
            
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
