/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

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
	class CVService;

	/**
	 * Configurable OpenCV library parameters.
	 */
	class NAPAPI CVServiceConfiguration : public ServiceConfiguration
	{
		RTTI_ENABLE(ServiceConfiguration)
	public:
		virtual rtti::TypeInfo getServiceType() const override	{ return RTTI_OF(CVService); }
		int mThreadCount = -1;									///< Property: 'ThreadCount' max number of threads to use, -1 = default
	};

	/**
	 * Manages the OpenCV library and is responsible for grabbing and forwarding frame data to capture components.
	 * Frames are only grabbed automatically when a frame is made available by a CVCaptureDevice and
	 * if a CVFrameCaptureComponent is interested in data from a capture device. If no capture component
	 * is interested in data from a capture device the frame will not be grabbed and decoded, saving potential
	 * compute cycles.
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

		/**
		 * Sets the max number of CPU threads to use. 
		 * 0 disables threading, all functions are run sequentially. 
		 * A value < 0 will reset the number of threads to the system default (all).
		 * @param count max number of CPU threads to use
		 */
		void setThreadCount(int count);

		/**
		 * @return the number of threads used by OpenCV for parallel regions.
		 */
		int getThreadCount() const;

	protected:

		/**
		* Initializes the video service
		* @param errorState contains the error message on failure
		* @return if the video service was initialized correctly
		*/
		virtual bool init(nap::utility::ErrorState& errorState) override;

		/**
		 * Grabs and forwards frame data to capture components.
		 * Frames are only grabbed when a frame is made available by a CVCaptureDevice and
		 * if a CVFrameCaptureComponent is interested in data from a capture device. If no capture component
		 * is interested in data from a capture device the frame will not be grabbed and decoded, saving potential
		 * compute cycles.
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