#pragma once

// Local Includes
#include "videoplayer.h"

// External Includes
#include <nap/service.h>

namespace nap
{
	/**
	 * Initializes the FFMPEG library (libavformat etc.) and registers all the codecs.
	 * This service also updates all system wide available video players
	 */
	class NAPAPI VideoService : public Service
	{
		friend class VideoPlayer;
		RTTI_ENABLE(Service)
	public:
		// Default constructor
		VideoService(ServiceConfiguration* configuration);

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
		void registerVideoPlayer(VideoPlayer& receiver);

		/**
		* Removes a video player from the service
		*/
		void removeVideoPlayer(VideoPlayer& receiver);

	private:
		std::vector<VideoPlayer*> mVideoPlayers;				///< All registered video players
		bool mVideoMaterialInitialized = false;					///< If the video material is properly initialized
	};
}