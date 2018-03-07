#pragma once

// Local Includes
#include "video.h"

// Nap Includes
#include <nap/service.h>
#include <queue>

namespace nap
{
	/**
	 * Initializes the FFMPEG library (libavformat etc.) and registers all the codecs.
	 * This service also updates all system wide available video players
	 */
	class NAPAPI VideoService : public Service
	{
		friend class Video;
		RTTI_ENABLE(Service)
	public:
		// Default constructor
		VideoService() = default;

		// Disable copy
		VideoService(const VideoService& that) = delete;
		VideoService& operator=(const VideoService&) = delete;

	protected:
		// This service depends on render and scene
		virtual void getDependentServices(std::vector<rtti::TypeInfo>& dependencies) override;

		/**
		* Initializes the video service
		* @param errorState contains the error message on failure
		* @return if the video service was initialized correctly
		*/
		virtual bool init(nap::utility::ErrorState& errorState) override;

		virtual void shutdown();

		/**
		 * Updates all registered video resources
		 */
		virtual void update(double deltaTime) override;

		/**
		 *	Object creators associated with video module
		 */
		virtual void registerObjectCreators(rtti::Factory& factory) override;

	private:
		/**
		* Registers a video player with the service
		*/
		void registerVideoPlayer(Video& receiver);

		/**
		* Removes a video player from the service
		*/
		void removeVideoPlayer(Video& receiver);

		std::unique_ptr<AudioFormat> audio_open(void *opaque, int wanted_nb_channels, int wanted_sample_rate);

		static void sdlAudioCallback(void* userData, uint8_t* stream, int len);

	private:
		std::vector<Video*> mVideoPlayers;					///< All registered video players
		std::unique_ptr<AudioFormat> mTargetAudioFormat;	///< Audio format that our device is using
	};
}