#pragma once

// Nap includes
#include <nap/resourceptr.h>

// Audio includes
#include <audio/core/audionodeptr.h>
#include <audio/core/audioobject.h>
#include <audio/node/bufferplayernode.h>
#include <audio/resource/audiobufferresource.h>

namespace nap
{
    
    namespace audio
    {
        
        /**
         * AudioObject to play back audio contained by an AudioBufferResource.
         */
        class BufferPlayer : public MultiChannelObject
        {
            RTTI_ENABLE(MultiChannelObject)
            
        public:
            BufferPlayer() = default;
            
            int mChannelCount = 1; /**< Number of channels that will be played back from the source buffer */
            ResourcePtr<AudioBufferResource> mBufferResource = nullptr; /**< Resource containing the buffer that will be played. */
            bool mAutoPlay = true; /**<  If true, the object will start playing back immediately after initialization. */
            
        private:
            NodePtr<Node> createNode(int channel, NodeManager& nodeManager) override
            {
                auto node = make_node<BufferPlayerNode>(nodeManager);
                if (mAutoPlay)
                    node->play(mBufferResource->getBuffer()[channel], 0, 1.);
                return std::move(node);
            }
            
            int getChannelCount() const override { return mChannelCount; }
        };
        
        
    }
    
}

