#pragma once

// Nap includes
#include <nap/resourceptr.h>

// Audio includes
#include <audio/core/nodeobject.h>
#include <audio/node/mixnode.h>

namespace nap
{
    
    namespace audio
    {
        
        class NAPAPI Mixer : public MultiChannel<MixNode>
        {
            RTTI_ENABLE(MultiChannelBase)
            
        public:
            Mixer() = default;
            
            std::vector<ResourcePtr<AudioObject>> mInputs;
            
        private:
            bool initNode(int channel, MixNode& node, utility::ErrorState& errorState) override
            {
                for (auto& input : mInputs)
                    if (input != nullptr)
                    {
                        node.inputs.connect(*input->getInstance()->getOutputForChannel(channel % input->getInstance()->getChannelCount()));
                    }
                return true;
            }
        };
        
        
    }
    
}

