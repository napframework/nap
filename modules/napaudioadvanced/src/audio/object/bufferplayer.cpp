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
        
        SafeOwner<Node> BufferPlayer::createNode(int channel, AudioService& audioService)
        {
            auto node = audioService.makeSafe<BufferPlayerNode>(audioService.getNodeManager());
            node->setBuffer(mBufferResource->getBuffer());
            if (mAutoPlay)
                node->play(0, 1.);
            return std::move(node);
        }
    }
    
}
