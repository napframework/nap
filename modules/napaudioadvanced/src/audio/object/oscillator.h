#pragma once

// Nap includes
#include <nap/resourceptr.h>

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
            Oscillator()
            {
                mWaveTable = std::make_shared<WaveTable>(2048);
            }
            
            int mChannelCount = 1;
            std::vector<ControllerValue> mFrequency = { 220.f };
            std::vector<ControllerValue> mAmplitude = { 1.f };
            ResourcePtr<AudioObject> mFmInput;
            
        private:
            NodePtr<Node> createNode(int channel, NodeManager& nodeManager) override
            {
                auto node = make_node<OscillatorNode>(nodeManager, mWaveTable);
                node->setFrequency(mFrequency[channel % mFrequency.size()]);
                node->setAmplitude(mAmplitude[channel % mAmplitude.size()]);
                if (mFmInput != nullptr)
                {
                    node->fmInput.connect(mFmInput->getInstance()->getOutputForChannel(channel % mFmInput->getInstance()->getChannelCount()));
                }
                
                return std::move(node);
            }
            
            int getChannelCount() const override { return mChannelCount; }
            std::shared_ptr<WaveTable> mWaveTable = nullptr;
        };
        
       
    }
    
}
