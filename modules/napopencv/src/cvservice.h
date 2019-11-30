#pragma once

// Nap Includes
#include <nap/service.h>

namespace nap
{
	/**
	 * Initializes the OpenCV library.
	 */
	class NAPAPI CVService : public Service
	{
		friend class Video;
		RTTI_ENABLE(Service)
	public:
		// Default constructor
		CVService(ServiceConfiguration* configuration);

		// Disable copy
		CVService(const CVService& that) = delete;
		CVService& operator=(const CVService&) = delete;

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
	};
}