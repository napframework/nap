#pragma once

// Local Includes
#include "cvcaptureapi.h"

// External Includes
#include <nap/device.h>
#include <nap/numeric.h>
#include <opencv2/videoio.hpp>
#include <opencv2/imgproc.hpp>

namespace nap
{
	/**
	 * Base class of all OpenCV video capture devices, including: video files, image sequences, cameras and web-streams.
	 * Override the various virtual functions to implement an OpenCV capture device.
	 */
	class NAPAPI CVVideoCapture : public Device
	{
		RTTI_ENABLE(Device)
	public:

		// Stops the device
		virtual ~CVVideoCapture() override;

		/**
		 * Starts the capture device.
		 * @param errorState contains the error if the device can't be started
		 * @return if the device started
		 */
		virtual bool start(utility::ErrorState& errorState) override final;

		/**
		 * Stops the capture device.
		 */
		virtual void stop() override final;

		/**
		 * Set an OpenCV capture device property.
		 * @param propID the property to set.
		 * @param value the new property value.
		 * @return if the property is applied.
		 */
		bool setProperty(cv::VideoCaptureProperties propID, double value);

		/**
		 * Get an OpenCV camera device property.
		 * @param propID the property to get.
		 * @return property value, if available.
		 */
		double getProperty(cv::VideoCaptureProperties propID) const;

		/**
		 * Returns the actual width of the frame in pixels.
		 * @return actual width of the frame in pixels
		 */
		int getWidth() const;

		/**
		 * Returns the actual height of the frame in pixels.
		 * @return actual height of the frame in pixels
		 */
		int getHeight() const;

		ECVCaptureAPI	mAPIPreference =  ECVCaptureAPI::Auto;	///< Property: 'Backend' the capture api preference, 0 = default. See cv::CVVideoCapture for a full list of options.

	protected:

		/**
		 * @return the OpenCV video capture device
		 */
		cv::VideoCapture& getCaptureDevice()					{ return mCaptureDevice; }

		/**
		* @return the OpenCV video capture device
		*/
		const cv::VideoCapture&	getCaptureDevice() const		{ return mCaptureDevice; }

		/**
		 * Needs to be implemented in a derived class.
		 * Called automatically by this device on startup when the OpenCV capture device needs to be opened.
		 * It is the responsibility of the derived class to open the device in this call and return success or failure.
		 * @param captureDevice handle to the device that needs to be opened.
		 * @param api the preferred backend video capture api
		 * @param error contains the error when the device could not be opened
		 * @return if the device opened correctly
		 */
		virtual bool onOpen(cv::VideoCapture& captureDevice, int api, nap::utility::ErrorState& error) = 0;

		/**
		 * Can be implemented in a derived class.
		 * Called automatically by this device on startup, when the capture device is open.
		 * @param error contains the error when the device can not be started
		 * @return if the device started correctly
		 */
		virtual bool onStart(cv::VideoCapture& captureDevice, utility::ErrorState& error)			{ return true; }

		/**
		 * Can be implemented in a derived class.
		 * Called automatically by this device on stop, before the capture device is released.
		 * @param error contains the error when the device can not be started
		 * @return if the device started correctly
		 */
		virtual void onStop()									{ }

	private:
		cv::VideoCapture	mCaptureDevice;			///< The open-cv video capture device
	};
}
