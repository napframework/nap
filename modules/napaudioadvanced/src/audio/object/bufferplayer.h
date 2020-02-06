#pragma once

// Nap includes
#include <nap/resourceptr.h>

// Audio includes
#include <audio/utility/safeptr.h>
#include <audio/core/nodeobject.h>
#include <audio/node/bufferplayernode.h>
#include <audio/resource/audiobufferresource.h>

namespace nap
{
    
    namespace audio
    {
        
        /**
         * AudioObject to play back audio contained by an AudioBufferResource.
         */
        class NAPAPI BufferPlayer : public MultiChannel<BufferPlayerNode>
        {
            RTTI_ENABLE(MultiChannelBase)
            
        public:
            BufferPlayer() = default;
            
            ResourcePtr<AudioBufferResource> mBufferResource = nullptr; /**< Resource containing the buffer that will be played. */
            bool mAutoPlay = true; /**<  If true, the object will start playing back immediately after initialization. */
            
        private:
            bool initNode(int channel, BufferPlayerNode& node, utility::ErrorState& errorState) override;
        };
        
        
    }
    
}

