#pragma once

// Local Includes
#include "cvcaptureapi.h"
#include "cvframe.h"

// External Includes
#include <nap/resource.h>
#include <opencv2/videoio.hpp>
#include <opencv2/imgproc.hpp>

namespace nap
{
	// Forward Declares
	class CVVideoCapture;

	/**
	 * Grabs and processes OpenCV video frames.
	 */
	class NAPAPI CVAdapter : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		virtual ~CVAdapter();

		/**
		 * Initialize this object after de-serialization
		 * @param errorState contains the error message when initialization fails
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * Sets an OpenCV capture property. This call is thread safe.
		 * The actual property is applied when the new frame is captured, not immediately.
		 * A new frame is queued immediately.
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
		 * Return false when decoding fails, otherwise return true.
		 * The 'outFrame' should contain the decoded frame on success.
		 * @param captureDevice the device to capture the frame from.
		 * @param outFrame contains the new decoded frame
		 * @return if decoding succeeded.
		 */
		virtual bool onRetrieve(cv::VideoCapture& captureDevice, CVFrame& outFrame, utility::ErrorState& error) = 0;

		/**
		 * Called automatically by this device on stop, after the capture device is released.
		 */
		virtual void onClose() { }

		/**
		 * Called right after the frame is stored.
		 */
		virtual void onCopy() { }

		/**
		 * @return the OpenCV video capture device
		 */
		cv::VideoCapture& getCaptureDevice()				{ return mCaptureDevice; }

		/**
		 * @return the OpenCV video capture device
		 */
		const cv::VideoCapture&	getCaptureDevice() const	{ return mCaptureDevice; }

	private:
		friend class CVVideoCapture;
		cv::VideoCapture  mCaptureDevice;					///< The open-cv video capture device

		bool open(nap::utility::ErrorState& error);
		
		void close()										{ onClose(); }

		bool retrieve(CVFrame& frame, utility::ErrorState& error);

		void copied()										{ onCopy(); }
	};
}
