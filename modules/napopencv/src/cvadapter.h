/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "cvcaptureapi.h"
#include "cvframe.h"
#include "cvcaptureerror.h"

// External Includes
#include <nap/device.h>
#include <opencv2/videoio.hpp>
#include <opencv2/imgproc.hpp>

namespace nap
{
	// Forward Declares
	class CVCaptureDevice;

	/**
	 * Interface for a specific type of OpenCV capture device (camera, video, network stream etc).
	 * Derive from this class to implement your own OpenCV adapter. The adapter itself does not schedule 
	 * the frame capture operation, that is handled in the background by the nap::CVCaptureDevice.
	 * That device (a-synchronously) handles the frame grab and retrieve operations.
	 * The capture device also opens the adapter on startup and closes it when stopped.
	 * Note that every adapter should only be added once to a specific nap::CVCaptureDevice!
	 * Restart or remove the adapter if it throws an error during the capture operation, errors are often caused
	 * by a connection failure or end of stream.
	 */
	class NAPAPI CVAdapter : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		/**
		 * Initializes the OpenCV Adapter
		 * @param errorState contains the error when initialization fails.
		 * @return if initialization succeeded.
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * Sets an OpenCV capture property. This call is thread safe.
		 * The actual property is applied when the new frame is captured, not immediately.
		 * This call has no effect when the adapter is not open or currently captured.
		 * It is not guaranteed that the property can be set,
		 * this depends completely on the interface exposed by the hardware.
		 * @param propID the property to set.
		 * @param value the new property value.
		 */
		void setProperty(cv::VideoCaptureProperties propID, double value);

		/**
		 * Get an OpenCV camera device property. Property is read immediately.
		 * Note that depending on your hardware this call can be slow!
		 * It is not guaranteed that the property can be read, 
		 * this depends completely on the interface exposed by the hardware.
		 * @param propID the property to get.
		 * @return property value if available.
		 */
		double getProperty(cv::VideoCaptureProperties propID) const;

		/**
		 * Checks if any errors are associated with this adapter.
		 * Note that an adapter can be 'open' and report an error at the same time.
		 * Set the 'CloseOnError' flag to ensure the device is closed by the capture device 
		 * when a capture error occurs.
		 *
		 * @return if any errors are associated with this adapter
		 */
		bool hasErrors() const;

		/**
		 * Returns all errors associated with this adapter.
		 * Only call this when hasErrors() returned true.
		 * @return all errors associated with this adapter.
		 */
		CVCaptureErrorMap getErrors() const;

		/**
		 * Returns if the device is currently open and ready for capturing.
		 * Note that in case of an error the device can still be open but not 
		 * capture any new frames. Set the 'CloseOnError' flag to ensure the device
		 * is closed by the capture device when a capture error occurs.
		 * Use this call in combination with hasErrors() to identify possible issues. 
		 *
		 * @return if the device is currently open and ready for processing.
		 */
		bool isOpen() const;

		/**
		 * Re-opens the OpenCV capture device and restarts the background capture process.
		 * If the restart procedure fails the adapter is removed from the capture process and
		 * an OpenError is set. When the restart procedure succeeds all errors are cleared.
		 * @param error contains the error if the restart procedure fails.
		 * @return if the restart procedure succeeded.
		 */
		bool restart(utility::ErrorState& error);

		/**
		 * @return number of OpenCV matrices associated with a single frame
		 */
		virtual int getMatrixCount() = 0;

		bool			mCloseOnCaptureError = false;			///< Property: 'CloseOnError' controls if the adapter is closed when it throws a frame grab error.
		ECVCaptureAPI	mAPIPreference = ECVCaptureAPI::Auto;	///< Property: 'Backend' the capture api preference, 0 = default. See cv::CVVideoCapture for a full list of options.

	protected:
		/**
		 * Needs to be implemented in a derived class.
		 * Called automatically by the nap::CVCaptureDevice on startup, before capture.
		 * @param captureDevice handle to the device that needs to be opened.
		 * @param api the preferred backend video capture api
		 * @param error contains the error when the device could not be opened
		 * @return if the device opened correctly
		 */
		virtual bool onOpen(cv::VideoCapture& captureDevice, int api, nap::utility::ErrorState& error) = 0;

		/**
		 * Called automatically by this device when stopped, after the device connection is closed.
		 */
		virtual void onClose() { }

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
		 * Called right after the frame is stored. Can be implemented in a derived class.
		 */
		virtual void onCopy()										{ }

		/**
		 * @return the OpenCV video capture device
		 */
		cv::VideoCapture& getCaptureDevice();

		/**
		 * @return the OpenCV video capture device
		 */
		const cv::VideoCapture&	getCaptureDevice() const;

		/**
		 * @return the video capture device this adapter belongs to.
		 */
		CVCaptureDevice& getParent() const;

	private:
		friend class CVCaptureDevice;

		/**
		 * Called by the CVCaptureDevice, opens the OpenCV capture device, 
		 * @param errorState contains the error if the device can't be opened
		 * @return if the device opened
		 */
		virtual bool open(utility::ErrorState& errorState) final;

		/**
		 * Called by the CVCaptureDevice, closes the OpenCV capture device,
		 */
		virtual void close() final;

		/**
		 * Called by the capture device to retrieve recently grabbed frame.
		 * @param error contains the error if the operation fails
		 * @return new frame, empty if operation fails.
		 */
		CVFrame retrieve(utility::ErrorState& error);

		/**
		 * Called by the capture device after successful grab and retrieve.
		 */
		void copied()											{ onCopy(); }

		CVCaptureDevice*	mParent;				///< The nap parent capture device
		cv::VideoCapture	mCaptureDevice;			///< The open-cv video capture device
	};
}
