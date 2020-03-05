#pragma once

// Nap includes
#include <nap/resourceptr.h>

// Audio includes
#include <audio/core/nodeobject.h>
#include <audio/node/inputnode.h>
#include <audio/utility/safeptr.h>

namespace nap
{
    
    namespace audio
    {
        
        /**
         * Multichannel oscillator object.
         */
        class NAPAPI Input : public MultiChannel<InputNode>
        {
            RTTI_ENABLE(MultiChannelBase)
            
        public:
            std::vector<int> mChannels = { 0 }; ///< property: 'Channels' Defines what audio input channels to receive data from. The size of this array determines the number of channels that this component will output.
            
        private:
            bool initNode(int channel, InputNode& node, utility::ErrorState& errorState) override
            {
                node.setInputChannel(mChannels[channel % mChannels.size()]);
                return true;
            }
        };
        
       
    }
    
}
