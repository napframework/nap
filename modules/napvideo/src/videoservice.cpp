/// local includes
#include "videoservice.h"

// External includes
#include <sceneservice.h>
#include <renderservice.h>

extern "C"
{
	#include <libavcodec/avcodec.h>
	#include <libavformat/avformat.h>
}

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::VideoService)
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
		{
			player->update(deltaTime);
		}
	}


	void VideoService::registerObjectCreators(rtti::Factory& factory)
	{
		factory.addObjectCreator(std::make_unique<VideoPlayerObjectCreator>(*this));
	}


	void VideoService::shutdown()
	{
		mVideoMaterial.reset(nullptr);
		mVideoShader.reset(nullptr);
	}


	ResourcePtr<Material> VideoService::getMaterial(utility::ErrorState& error)
	{
		// Create video material if doesn't exist
		if (mVideoMaterial == nullptr)
		{
			// Create video shader and material instance
			mVideoShader = std::make_unique<VideoShader>(getCore());
			mVideoMaterial = std::make_unique<Material>(getCore());
			mVideoMaterial->mShader = ResourcePtr<Shader>(mVideoShader.get());

			// Initialize shader and material
			if (mVideoShader->init(error) && mVideoMaterial->init(error))
				mVideoMaterialInitialized = true;
		}

		// Ensure video material initialized correctly
		if (!error.check(mVideoMaterialInitialized, "Video material initialization failed"))
			return nullptr;

		return nap::ResourcePtr<Material>(mVideoMaterial.get());
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

