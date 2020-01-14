#pragma once

// Local Includes
#include "cvcaptureapi.h"
#include "cvframe.h"

// External Includes
#include <nap/device.h>
#include <opencv2/videoio.hpp>
#include <opencv2/imgproc.hpp>

namespace nap
{
	// Forward Declares
	class CVVideoCapture;

	/**
	 * OpenCV capture device interface. Every device is opened on startup and closed on stop. 
	 * Derive from this class to implement your own OpenCV capture device.
	 * The device itself does not perform the frame capture operation, this is handled in the background by
	 * the nap::CVVideoCaptureDevice. That device (a-synchronously) handles the frame grab and retrieve operations.
	 */
	class NAPAPI CVAdapter : public Device
	{
		RTTI_ENABLE(Device)
	public:
		/**
		 * Sets an OpenCV capture property. This call is thread safe.
		 * The actual property is applied when the new frame is captured, not immediately.
		 * @param propID the property to set.
		 * @param value the new property value.
		 */
		void setProperty(cv::VideoCaptureProperties propID, double value);

		/**
		 * Get an OpenCV camera device property. Property is read immediately.
		 * Note that depending on your hardware this call can be slow!
		 * @param propID the property to get.
		 * @return property value, if available.
		 */
		double getProperty(cv::VideoCaptureProperties propID) const;

		/**
		 * @return if the device is currently open and ready for processing.
		 */
		bool started() const															{ return mStarted; }

		/**
		 * Starts the OpenCV capture device. During startup the device is opened.
		 * @param errorState contains the error if the device can't be started
		 * @return if the device started
		 */
		virtual bool start(utility::ErrorState& errorState) override final;

		/**
		 * Stops and closes the OpenCV capture device.
		 */
		virtual void stop() override final;

		ECVCaptureAPI	mAPIPreference = ECVCaptureAPI::Auto;	///< Property: 'Backend' the capture api preference, 0 = default. See cv::CVVideoCapture for a full list of options.

	protected:

		/**
		 * @return number of OpenCV matrices associated with a single frame
		 */
		virtual int getMatrixCount() = 0;

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
		 * This method decodes and returns the just grabbed frame.
		 * Needs to be implemented in a derived class. 
		 * An empty return frame is interpreted as a decoding error by the processing device.
		 * @param captureDevice the device to capture the frame from.
		 * @param outFrame contains the new decoded frame
		 * @return the decoded frame, return an empty frame on failure.
		 */
		virtual CVFrame onRetrieve(cv::VideoCapture& captureDevice, utility::ErrorState& error) = 0;

		/**
		 * Called automatically by this device when stopped, after the device connection is closed.
		 */
		virtual void onClose() { }

		/**
		 * Called right after the frame is stored.
		 */
		virtual void onCopy() { }

		/**
		 * @return the OpenCV video capture device
		 */
		cv::VideoCapture& getCaptureDevice();

		/**
		 * @return the OpenCV video capture device
		 */
		const cv::VideoCapture&	getCaptureDevice() const;

	protected:
		/**
		 * @return video capture device this adapter belongs to
		 */
		CVVideoCapture& getParent() const;

	private:
		friend class CVVideoCapture;
		CVVideoCapture*		mParent;							///< The nap parent capture device
		cv::VideoCapture	mCaptureDevice;						///< The open-cv video capture device
		bool				mStarted = false;					///< If the video capture device started

		CVFrame retrieve(utility::ErrorState& error);
		void copied()											{ onCopy(); }
	};
}
