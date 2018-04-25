#pragma once

// Local Includes
#include "video.h"

// Nap Includes
#include <nap/service.h>

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
		VideoService(ServiceConfiguration* configuration);

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

	private:
		std::vector<Video*> mVideoPlayers;			///< All registered video players
	};
}