#pragma once

// Local Includes
#include "videoplayer.h"
#include "videoshader.h"

// External Includes
#include <nap/service.h>
#include <material.h>

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

		/**
		 * Returns a video material that can be shared.
		 * The material points to a shader that converts YUV textures into an RGB image.
		 * The material is created when requested for the first time.
		 * Use this material as a template for a video material instance. 
		 * @param error contains the error if the material could not be created.
		 * @return shared video material, nullptr if shader or material could not be created or failed to initialize.
		 */
		nap::ResourcePtr<Material> getMaterial(utility::ErrorState& error);


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

		/**
		 * Invoked when exiting the main loop, after app shutdown is called
		 * Use this function to close service specific handles, drivers or devices
		 * When service B depends on A, Service B is shutdown before A
		 */
		virtual void shutdown() override;

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
		std::unique_ptr<VideoShader> mVideoShader = nullptr;	///< Sharable video shader
		std::unique_ptr<Material> mVideoMaterial = nullptr;		///< Sharable video material
		bool mVideoMaterialInitialized = false;					///< If the video material is properly initialized
	};
}