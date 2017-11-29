#pragma once

// Nap includes
#include <nap/objectptr.h>

// Audio includes
#include <audio/core/audioobject.h>
#include <audio/node/bufferplayernode.h>
#include <audio/resource/audiobufferresource.h>

namespace nap
{
    
    namespace audio
    {
        
        class BufferPlayer : public MultiChannelObject
        {
            RTTI_ENABLE(MultiChannelObject)
            
        public:
            BufferPlayer() = default;
            
            int mChannelCount = 1;
            ObjectPtr<AudioBufferResource> mBufferResource = nullptr;
            bool mAutoPlay = true;
            
        private:
            std::unique_ptr<Node> createNode(int channel, NodeManager& nodeManager) override
            {
                auto node = std::make_unique<BufferPlayerNode>(nodeManager);
                if (mAutoPlay)
                    node->play(mBufferResource->getBuffer()[channel], 0, 1.);
                return std::move(node);
            }
            
            int getChannelCount() const override { return mChannelCount; }
        };
        
        
    }
    
}

