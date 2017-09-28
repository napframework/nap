#pragma once

// Nap includes
#include <nap/objectptr.h>

// Audio includes
#include <object/audioobject.h>
#include <node/oscillatornode.h>

namespace nap {
    
    namespace audio {
        
        class Oscillator : public MultiChannelObject {
            RTTI_ENABLE(MultiChannelObject)
            
        public:
            Oscillator() = default;
            Oscillator(NodeManager& nodeManager) : MultiChannelObject(nodeManager) { }
            
            int mChannelCount = 1;
            std::vector<ControllerValue> mFrequency = { 220.f };
            std::vector<ControllerValue> mAmplitude = { 1.f };
            ObjectPtr<AudioObject> mFmInput;
            
        private:
            std::unique_ptr<Node> createNode(int channel) override
            {
                auto node = std::make_unique<OscillatorNode>(getNodeManager(), mWaveTable);
                node->setFrequency(mFrequency[channel % mFrequency.size()]);
                node->setAmplitude(mAmplitude[channel % mAmplitude.size()]);
                if (mFmInput != nullptr)
                {
                    node->fmInput.connect(mFmInput->getInstance()->getOutputForChannel(channel % mFmInput->getInstance()->getChannelCount()));
                }
                
                return node;
            }
            
            int getChannelCount() const override { return mChannelCount; }
            
            WaveTable mWaveTable  = { 2048 };
        };
        
        using OscillatorCreator = rtti::ObjectCreator<Oscillator, NodeManager>;
    }
    
}
