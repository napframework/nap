#pragma once

// Local Includes
#include "cvcaptureapi.h"

// External Includes
#include <nap/device.h>
#include <nap/numeric.h>
#include <opencv2/videoio.hpp>
#include <opencv2/imgproc.hpp>
#include <thread>
#include <future>
#include <atomic>

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
		 * Grabs the last captured frame if new. The result is stored in the given target.
		 * Internally the frame is removed from the queue.
		 * If there is no new capture the target is not updated and the function returns false.
		 * This operation hands over ownership of the captured frame.
		 * This call is thread safe and can be called every frame in to check for a new frame.
		 * @param target updated to hold the last captured frame data if a new frame is available.
		 * @return if the target is updated with the contents of a new captured frame.
		 */
		bool grab(cv::UMat& target);

		/**
		 * Tells the capture thread to capture the next available frame.
		 * Use grab() to check if the new frame is available. 
		 * Typically you call capture after a successful frame grab operation.
 		 */
		void capture();

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
		 * Sets an OpenCV capture property thread safe.
		 * The actual property is applied when the new frame is captured, not immediately.
		 * A new frame is queued.
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
		 * Returns the OpenCV capture device frame width.
		 * @return the OpenCV capture device frame width in pixels.
		 */
		int getWidth() const;

		/**
		 * Returns the OpenCV capture device frame height.
		 * @return the OpenCV capture device frame height in pixels.
		 */
		int getHeight() const;

		/**
		 * @return the OpenCV video capture device
		 */
		cv::VideoCapture& getCaptureDevice()					{ return mCaptureDevice; }

		/**
		 * @return the OpenCV video capture device
		 */
		const cv::VideoCapture&	getCaptureDevice() const		{ return mCaptureDevice; }

		ECVCaptureAPI	mAPIPreference =  ECVCaptureAPI::Auto;	///< Property: 'Backend' the capture api preference, 0 = default. See cv::CVVideoCapture for a full list of options.
		bool			mConvertRGB = true;						///< Property: 'ConvertRGB' if the frame is converted into RGB
		bool			mFlipHorizontal = false;				///< Property: 'FlipHorizontal' flips the frame on the x-axis
		bool			mFlipVertical = false;					///< Property: 'FlipVertical' flips the frame on the y-axis

	protected:
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
		 * Called automatically by this device on stop, before the capture device is released.
		 * @param error contains the error when the device can not be started
		 * @return if the device started correctly
		 */
		virtual void onClose()									{ }

		/**
		 * This method decodes and returns the just grabbed frame.
		 * Needs to be implemented in a derived class. 
		 * Return false when decoding fails, otherwise return true.
		 * The 'outFrame' should contain the decoded frame on success.
		 * @param captureDevice the device to capture the frame from.
		 * @param outFrame contains the new decoded frame
		 * @return if decoding succeeded.
		 */
		virtual bool onRetrieve(cv::VideoCapture& captureDevice, cv::UMat& outFrame, utility::ErrorState& error) = 0;

		/**
		 * Called right after the frame is stored.
		 */
		virtual void onCopy()									{ }

	private:
		cv::VideoCapture		mCaptureDevice;					///< The open-cv video capture device
		cv::UMat				mCaptureMat;					///< The GPU / CPU matrix that holds the most recent captured video frame
		bool					mCaptureFrame	= true;			///< Proceed to next frame
		std::atomic<bool>		mFrameAvailable = { false };	///< If a new frame is captured

		std::future<void>		mCaptureTask;					///< The thread that monitor the read thread
		std::mutex				mCaptureMutex;					///< The mutex that safe guards the capture thread
		bool					mStopCapturing = false;			///< Signals the capture thread to stop capturing video
		std::condition_variable	mCaptureCondition;				///< Used for telling the polling task to continue

		// Properties that are set when a new frame is grabbed
		std::unordered_map<int, double> mProperties;
		bool mSetProperties = false;

		/**
		 * Captures new frames.
		 */
		void captureTask();

	};
}
