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

RTTI_DEFINE_CLASS(nap::VideoService)

namespace nap
{
	int VideoService::audio_open(void *userData, int64_t wanted_channel_layout, int wanted_nb_channels, int wanted_sample_rate, AudioParams& audio_hw_params)
	{
		SDL_AudioSpec wanted_spec, spec;
		const char *env;
		static const int next_nb_channels[] = { 0, 0, 1, 6, 2, 6, 4, 6 };
		static const int next_sample_rates[] = { 0, 44100, 48000, 96000, 192000 };
		int next_sample_rate_idx = FF_ARRAY_ELEMS(next_sample_rates) - 1;

		env = SDL_getenv("SDL_AUDIO_CHANNELS");
		if (env) {
			wanted_nb_channels = atoi(env);
			wanted_channel_layout = av_get_default_channel_layout(wanted_nb_channels);
		}
		if (!wanted_channel_layout || wanted_nb_channels != av_get_channel_layout_nb_channels(wanted_channel_layout)) {
			wanted_channel_layout = av_get_default_channel_layout(wanted_nb_channels);
			wanted_channel_layout &= ~AV_CH_LAYOUT_STEREO_DOWNMIX;
		}
		wanted_nb_channels = av_get_channel_layout_nb_channels(wanted_channel_layout);
		wanted_spec.channels = wanted_nb_channels;
		wanted_spec.freq = wanted_sample_rate;
		if (wanted_spec.freq <= 0 || wanted_spec.channels <= 0) {
			av_log(NULL, AV_LOG_ERROR, "Invalid sample rate or channel count!\n");
			return -1;
		}
		while (next_sample_rate_idx && next_sample_rates[next_sample_rate_idx] >= wanted_spec.freq)
			next_sample_rate_idx--;
		wanted_spec.format = AUDIO_S16SYS;
		wanted_spec.silence = 0;
		wanted_spec.samples = FFMAX(512, 2 << av_log2(wanted_spec.freq / 30));
		wanted_spec.callback = &VideoService::sdlAudioCallback;
		wanted_spec.userdata = userData;
		while (SDL_OpenAudio(&wanted_spec, &spec) < 0) {
			av_log(NULL, AV_LOG_WARNING, "SDL_OpenAudio (%d channels, %d Hz): %s\n",
				wanted_spec.channels, wanted_spec.freq, SDL_GetError());
			wanted_spec.channels = next_nb_channels[FFMIN(7, wanted_spec.channels)];
			if (!wanted_spec.channels) {
				wanted_spec.freq = next_sample_rates[next_sample_rate_idx--];
				wanted_spec.channels = wanted_nb_channels;
				if (!wanted_spec.freq) {
					av_log(NULL, AV_LOG_ERROR,
						"No more combinations to try, audio open failed\n");
					return -1;
				}
			}
			wanted_channel_layout = av_get_default_channel_layout(wanted_spec.channels);
		}
		if (spec.format != AUDIO_S16SYS) {
			av_log(NULL, AV_LOG_ERROR,
				"SDL advised audio format %d is not supported!\n", spec.format);
			return -1;
		}
		if (spec.channels != wanted_spec.channels) {
			wanted_channel_layout = av_get_default_channel_layout(spec.channels);
			if (!wanted_channel_layout) {
				av_log(NULL, AV_LOG_ERROR,
					"SDL advised channel count %d is not supported!\n", spec.channels);
				return -1;
			}
		}

		audio_hw_params.fmt = AV_SAMPLE_FMT_S16;
		audio_hw_params.freq = spec.freq;
		audio_hw_params.channel_layout = wanted_channel_layout;
		audio_hw_params.channels = spec.channels;
		audio_hw_params.frame_size = av_samples_get_buffer_size(NULL, audio_hw_params.channels, 1, (AVSampleFormat)audio_hw_params.fmt, 1);
		audio_hw_params.bytes_per_sec = av_samples_get_buffer_size(NULL, audio_hw_params.channels, audio_hw_params.freq, (AVSampleFormat)audio_hw_params.fmt, 1);
		assert(audio_hw_params.bytes_per_sec > 0 && audio_hw_params.frame_size > 0);

		return spec.size;
	}


	void VideoService::sdlAudioCallback(void* userData, uint8_t* stream, int len)
	{
		VideoService* video_service = (VideoService*)userData;

		Video* playing_video = nullptr;
		for (Video* video : video_service->mVideoPlayers)
		{
			if (video->isPlaying())
			{
				playing_video = video;
				break;
			}
		}

		if (playing_video != nullptr)
			playing_video->OnAudioCallback(stream, len, video_service->mAudioHwParams);
	}


	bool VideoService::init(nap::utility::ErrorState& errorState)
	{
		av_register_all();
		avcodec_register_all();

		audio_open(this, 0, 2, 48000, mAudioHwParams);
		SDL_PauseAudio(0);

		return true;
	}


	void VideoService::shutdown()
	{
		SDL_CloseAudio();
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

