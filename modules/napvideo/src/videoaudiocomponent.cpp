#include "videoaudiocomponent.h"

// Nap includes
#include <entity.h>

// RTTI
RTTI_BEGIN_CLASS(nap::audio::VideoAudioComponent)
    RTTI_PROPERTY("Video", &nap::audio::VideoAudioComponent::mVideo, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("ChannelCount", &nap::audio::VideoAudioComponent::mChannelCount, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::VideoAudioComponentInstance)
    RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

namespace nap
{
    
    namespace audio
    {
    
        bool VideoAudioComponentInstance::init(utility::ErrorState& errorState)
        {
            auto resource = getComponent<VideoAudioComponent>();
            mNode = std::make_unique<VideoNode>(getNodeManager(), *resource->mVideo, resource->mChannelCount);
            return true;
        }
        

        void VideoAudioComponentInstance::setVideo(Video& video)
        {
            mNode->setVideo(video);
        }

        
    }
    
}
