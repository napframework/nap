#pragma once

// Nap Includes
#include <nap/service.h>
#include <nap/signalslot.h>

namespace nap
{
	// Forward Declares
	class CVCaptureDevice;
	class CVFrameEvent;
	class CVCaptureComponentInstance;

	/**
	 * Initializes the OpenCV library.
	 */
	class NAPAPI CVService : public Service
	{
		friend class CVCaptureDevice;
		friend class CVCaptureComponentInstance;
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

		/**
		 * Registers a cv capture device with the service
		 */
		void registerCaptureDevice(CVCaptureDevice& device);

		/**
		 * Unregisters a cv capture device from the service
		 */
		void removeCaptureDevice(CVCaptureDevice& device);

		/**
		 *	Registers a CV capture component
		 */
		void registerCaptureComponent(CVCaptureComponentInstance& input);

		/**
		 *	Removes a CV capture component from the service
		 */
		void removeCaptureComponent(CVCaptureComponentInstance& input);

	private:
		// All the web socket servers currently registered in the system
		std::vector<CVCaptureDevice*> mCaptureDevices;

		// All the capture components currently registered in the system
		std::vector<CVCaptureComponentInstance*> mCaptureComponents;
	};
}