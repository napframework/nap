/// local includes
#include "videoservice.h"

// External includes
#include <sceneservice.h>
#include <renderservice.h>
#include <nap/logger.h>

extern "C"
{
	#include <libavcodec/avcodec.h>
	#include <libavformat/avformat.h>
}

RTTI_DEFINE(nap::VideoService)

namespace nap
{
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
		{
			if (!player->update(deltaTime, error))
			{
				nap::Logger::warn(error.toString().c_str());
			}
		}
	}


	void VideoService::registerObjectCreators(rtti::Factory& factory)
	{
		factory.addObjectCreator(std::make_unique<VideoObjectCreator>(*this));
	}


	void VideoService::registerVideoPlayer(Video& receiver)
	{
		mVideoPlayers.emplace_back(&receiver);
	}


	void VideoService::removeVideoPlayer(Video& receiver)
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

