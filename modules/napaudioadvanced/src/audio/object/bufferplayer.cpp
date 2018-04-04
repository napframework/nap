#include "bufferplayer.h"

RTTI_BEGIN_CLASS(nap::audio::BufferPlayer)
    RTTI_PROPERTY("ChannelCount", &nap::audio::BufferPlayer::mChannelCount, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("AutoPlay", &nap::audio::BufferPlayer::mAutoPlay, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("Buffer", &nap::audio::BufferPlayer::mBufferResource, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

namespace nap
{
    
    namespace audio
    {
        
        NodePtr<Node> BufferPlayer::createNode(int channel, NodeManager& nodeManager)
        {
            auto node = make_node<BufferPlayerNode>(nodeManager);
            if (mAutoPlay)
                node->play(mBufferResource->getBuffer().getChannelPtr(channel), 0, 1.);
            return std::move(node);
        }
    }
    
}
