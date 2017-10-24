#pragma once

// Nap Includes
#include <nap/service.h>
#include <queue>

namespace nap
{
	/**
	 * 
	 */
	class NAPAPI VideoService : public Service
	{
		RTTI_ENABLE(Service)

	public:
		// Default constructor
		VideoService() = default;

		// Disable copy
		VideoService(const VideoService& that) = delete;
		VideoService& operator=(const VideoService&) = delete;

	protected:
		// This service depends on render and scene
		virtual void getDependencies(std::vector<rtti::TypeInfo>& dependencies) override;

		/**
		* Initializes the video service
		* @param errorState contains the error message on failure
		* @return if the video service was initialized correctly
		*/
		virtual bool init(nap::utility::ErrorState& errorState) override;
	};
}