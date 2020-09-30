/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "videoaudiocomponent.h"

// External Includes
#include <entity.h>
#include <audio/service/audioservice.h>

// RTTI
RTTI_BEGIN_CLASS(nap::audio::VideoAudioComponent)
    RTTI_PROPERTY("VideoPlayer",	&nap::audio::VideoAudioComponent::mVideoPlayer,		nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("ChannelCount",	&nap::audio::VideoAudioComponent::mChannelCount,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("ProcessAudio",	&nap::audio::VideoAudioComponent::mProcessAudio,	nap::rtti::EPropertyMetaData::Default)
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
			// Create resources
            audio::VideoAudioComponent* resource = getComponent<VideoAudioComponent>();
            audio::AudioService& audioService = getAudioService();
            mNode = audioService.makeSafe<VideoNode>(audioService.getNodeManager(), resource->mChannelCount, resource->mProcessAudio);

			// Listen to video changes
			mVideoPlayer = resource->mVideoPlayer.get();
			mVideoPlayer->VideoChanged.connect(mVideoChangedSlot);
			selectVideo(*mVideoPlayer);
            return true;
        }
        

        void VideoAudioComponentInstance::selectVideo(VideoPlayer& player)
        {
            mNode->setVideo(player.getVideo());
        }
    }
    
}
