#pragma once

// External Includes
#include <nap/device.h>
#include <opencv2/videoio.hpp>
#include <opencv2/imgproc.hpp>
#include <nap/numeric.h>
#include <thread>
#include <future>
#include <atomic>
#include <queue>

namespace nap
{
	/**
	 * Captures a video stream from a web cam or other peripheral video capture device.
	 * The captured video frame is stored on the GPU when hardware acceleration is available (OpenCL).
	 * Otherwise the captured video frame is stored on the CPU.
	 * This device captures the video stream on a background thread, call grab() to grab the last recorded video frame.
	 */
	class NAPAPI CVVideoCapture : public Device
	{
		RTTI_ENABLE(Device)
	public:

		// Stops the device
		virtual ~CVVideoCapture() override;

		/**
		* Initialize this object after de-serialization
		* @param errorState contains the error message when initialization fails
		*/
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * Starts the device
		 * @param errorState contains the error if the device can't be started
		 * @return if the device started
		 */
		virtual bool start(utility::ErrorState& errorState) override;

		/**
		 * Stops the device
		 */
		virtual void stop() override;

		/**
		 * Grabs the last captured frame if new, the result is stored in the given target.
		 * Internally the frame is removed from the queue.
		 * If there is no new capture the target is not updated and the function returns false.
		 * This call is thread safe and can be called every frame. 
		 * @param target holds the last captured frame if new.
		 * @return if the target is updated with the contents of a new captured frame.
		 */
		bool grab(cv::UMat& target);

		nap::uint			mDeviceIndex = 0;		///< Property: 'DeviceIndex' Capture device index
		nap::uint			mFrameWidth = 640;		///< Property: 'FrameWidth' width of the frame in pixels
		nap::uint			mFrameHeight = 480;		///< Property: 'FrameHeight' height of the frame in pixels

	private:
		cv::VideoCapture	mCaptureDevice;			///< The open-cv video capture device
		cv::UMat			mCaptureMat;			///< The GPU / CPU matrix that holds the most recent captured video frame
		bool				mNewFrame =  false ;	///< If a new frame is captured

		std::future<void>	mCaptureTask;			///< The thread that monitor the read thread
		std::mutex			mCaptureMutex;			///< The mutex that safe guards the capture thread
		bool				mStopCapturing = false;	///< Signals the capture thread to stop capturing video

		void capture();

		// Queue of captured frames
		std::queue<cv::Mat> mFrameQueue;
	};
}
