/// local includes
#include "videoservice.h"

// External includes
#include <sceneservice.h>
#include <renderservice.h>
#include <nap/logger.h>
#include "utility/datetimeutils.h"

extern "C"
{
	#include <libavcodec/avcodec.h>
	#include <libavformat/avformat.h>
}

#define DEBUG_LOG_TO_FILE	0

#if DEBUG_LOG_TO_FILE
	static FILE* debugFile = nullptr;
	static nap::utility::HighResolutionTimer debugFileTimer;
	static const char* debugFilename = "d:\\test.raw";
#endif

RTTI_DEFINE_CLASS(nap::VideoService)

namespace nap
{
	std::unique_ptr<AudioFormat> VideoService::audio_open(void *userData, int wanted_nb_channels, int wanted_sample_rate)
	{
		SDL_AudioSpec wanted_spec, spec;
		static const int next_nb_channels[] = { 0, 0, 1, 6, 2, 6, 4, 6 };
		static const int next_sample_rates[] = { 0, 44100, 48000, 96000, 192000 };
		int next_sample_rate_idx = FF_ARRAY_ELEMS(next_sample_rates) - 1;

		wanted_spec.channels = wanted_nb_channels;
		wanted_spec.freq = wanted_sample_rate;
		if (wanted_spec.freq <= 0 || wanted_spec.channels <= 0) {
			av_log(NULL, AV_LOG_ERROR, "Invalid sample rate or channel count!\n");
			return nullptr;
		}
		while (next_sample_rate_idx && next_sample_rates[next_sample_rate_idx] >= wanted_spec.freq)
			next_sample_rate_idx--;
		wanted_spec.format = AUDIO_S16SYS;
		wanted_spec.silence = 0;
		wanted_spec.samples = FFMAX(512, 2 << av_log2(wanted_spec.freq / 30));
		wanted_spec.callback = &VideoService::sdlAudioCallback;
		wanted_spec.userdata = userData;
		while (SDL_OpenAudio(&wanted_spec, &spec) < 0) 
		{
			av_log(NULL, AV_LOG_WARNING, "SDL_OpenAudio (%d channels, %d Hz): %s\n",
				wanted_spec.channels, wanted_spec.freq, SDL_GetError());
			wanted_spec.channels = next_nb_channels[FFMIN(7, wanted_spec.channels)];
			if (!wanted_spec.channels) 
			{
				wanted_spec.freq = next_sample_rates[next_sample_rate_idx--];
				wanted_spec.channels = wanted_nb_channels;
				if (!wanted_spec.freq) 
				{
					av_log(NULL, AV_LOG_ERROR,
						"No more combinations to try, audio open failed\n");
					return nullptr;
				}
			}
		}
		if (spec.format != AUDIO_S16SYS) 
		{
			av_log(NULL, AV_LOG_ERROR,
				"SDL advised audio format %d is not supported!\n", spec.format);
			return nullptr;
		}

		return std::make_unique<AudioFormat>(spec.channels, AudioFormat::ESampleFormat::S16, spec.freq);
	}


	void VideoService::sdlAudioCallback(void* userData, uint8_t* stream, int len)
	{
		VideoService* video_service = (VideoService*)userData;

		Video* playing_video = nullptr;
		for (Video* video : video_service->mVideoPlayers)
		{
			if (video->isPlaying() && video->hasAudio())
			{
				playing_video = video;
				break;
			}
		}

		bool hasAudio = false;
		if (playing_video != nullptr)
			hasAudio = playing_video->OnAudioCallback(stream, len, *video_service->mTargetAudioFormat);
				
		if (!hasAudio)
			memset(stream, 0, len);

#if DEBUG_LOG_TO_FILE
		if (debugFile != nullptr)
		{
			fwrite(stream, len, 1, debugFile);

			if (debugFileTimer.getElapsedTime() > 30.0)
			{
				fclose(debugFile);
				debugFile = nullptr;
			}
		}
#endif // DEBUG_LOG_TO_FILE
	}


	bool VideoService::init(nap::utility::ErrorState& errorState)
	{
		av_register_all();
		avcodec_register_all();

#if DEBUG_LOG_TO_FILE
		debugFile = fopen(debugFilename, "wb");
		debugFileTimer.start();
#endif // DEBUG_LOG_TO_FILE

//        mTargetAudioFormat = audio_open(this, 2, 48000);
//        SDL_PauseAudio(0);

		return true;
	}


	void VideoService::shutdown()
	{
//        SDL_CloseAudio();
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

