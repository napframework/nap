#pragma once

// Nap includes
#include <rtti/objectptr.h>

// Audio includes
#include <audio/core/audioobject.h>
#include <audio/node/oscillatornode.h>

namespace nap
{
    
    namespace audio
    {
        
        class Oscillator : public MultiChannelObject
        {
            RTTI_ENABLE(MultiChannelObject)
            
        public:
            Oscillator() = default;
            
            int mChannelCount = 1;
            std::vector<ControllerValue> mFrequency = { 220.f };
            std::vector<ControllerValue> mAmplitude = { 1.f };
            rtti::ObjectPtr<AudioObject> mFmInput;
            
        private:
            std::unique_ptr<Node> createNode(int channel, NodeManager& nodeManager) override
            {
                auto node = std::make_unique<OscillatorNode>(nodeManager, mWaveTable);
                node->setFrequency(mFrequency[channel % mFrequency.size()]);
                node->setAmplitude(mAmplitude[channel % mAmplitude.size()]);
                if (mFmInput != nullptr)
                {
                    node->fmInput.connect(mFmInput->getInstance()->getOutputForChannel(channel % mFmInput->getInstance()->getChannelCount()));
                }
                
                return std::move(node);
            }
            
            int getChannelCount() const override { return mChannelCount; }
            WaveTable mWaveTable  = { 2048 };
        };
        
       
    }
    
}
