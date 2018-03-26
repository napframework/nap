#pragma once

// Audio includes
#include <audio/core/audioobject.h>
#include <audio/node/gainnode.h>
#include <nap/resourceptr.h>

namespace nap
{
    
    namespace audio
    {
        
        class Gain : public MultiChannelObject
        {
            RTTI_ENABLE(MultiChannelObject)
            
        public:
            Gain() = default;
            
            int mChannelCount = 1;
            std::vector<ControllerValue> mGain = { 1.f };
            std::vector<ResourcePtr<AudioObject>> mInputs;
            
        private:
            NodePtr<Node> createNode(int channel, NodeManager& nodeManager) override
            {
                auto node = make_node<GainNode>(nodeManager);
                node->setGain(mGain[channel % mGain.size()]);
                for (auto& input : mInputs)
                    if (input != nullptr)
                    {
                        node->inputs.connect(input->getInstance()->getOutputForChannel(channel % input->getInstance()->getChannelCount()));
                    }
                
                return std::move(node);
            }
            
            int getChannelCount() const override { return mChannelCount; }
        };
        
        
    }
    
}

