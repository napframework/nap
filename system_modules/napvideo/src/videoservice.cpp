/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

/// local includes
#include "videoservice.h"

// External includes
#include <sceneservice.h>
#include <renderservice.h>
#include <nap/core.h>
#include <mathutils.h>

extern "C"
{
	#include <libavcodec/avcodec.h>
	#include <libavformat/avformat.h>
}

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::VideoService, "Initializes the FFMPEG library (libavformat etc.) and registers all the codecs")
	RTTI_CONSTRUCTOR(nap::ServiceConfiguration*)
RTTI_END_CLASS

namespace nap
{
	VideoService::VideoService(ServiceConfiguration* configuration) :
		Service(configuration)
	{
	}


	bool VideoService::init(nap::utility::ErrorState& errorState)
	{
		av_register_all();
		avcodec_register_all();
		return true;
	}


	void VideoService::update(double deltaTime)
	{
		nap::utility::ErrorState error;
		for (auto& player : mVideoPlayers)
			player->update(deltaTime);
	}


	void VideoService::registerObjectCreators(rtti::Factory& factory)
	{
		factory.addObjectCreator(std::make_unique<VideoPlayerObjectCreator>(*this));
	}


	void VideoService::registerVideoPlayer(VideoPlayer& receiver)
	{
		mVideoPlayers.emplace_back(&receiver);
	}


	void VideoService::removeVideoPlayer(VideoPlayer& receiver)
	{
		auto found_it = std::find_if(mVideoPlayers.begin(), mVideoPlayers.end(), [&](const auto& it)
		{
			return it == &receiver;
		});
		assert(found_it != mVideoPlayers.end());
		mVideoPlayers.erase(found_it);
	}


	void VideoService::getDependentServices(std::vector<rtti::TypeInfo>& dependencies)
	{
		dependencies.emplace_back(RTTI_OF(SceneService));
		dependencies.emplace_back(RTTI_OF(RenderService));
	}
}

