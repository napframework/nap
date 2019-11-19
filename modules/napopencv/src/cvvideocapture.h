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
	 * This device captures the video stream on a background thread. 
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
		 * Copies the last read captured frame.
		 * @param target the 
		 */
		void copy(cv::UMat& target);

		/**
		 * @return if a new frame is available
		 */
		bool hasNewFrame();

		nap::uint			mDeviceIndex = 0;		///< Property: 'DeviceIndex' Capture device index

	private:
		cv::VideoCapture	mCaptureDevice;			///< The open-cv video capture device
		cv::UMat			mCaptureMat;			///< The GPU / CPU matrix that holds the most recent captured video frame
		std::atomic<bool>	mNewFrame = { false };	///< If a new frame is captured

		std::future<void>	mCaptureTask;			///< The thread that monitor the read thread
		std::mutex			mCaptureMutex;			///< The mutex that safe guards the capture thread
		bool				mStopCapturing = false;	///< Signals the capture thread to stop capturing video

		void capture();

		// Queue of captured frames
		std::queue<cv::Mat> mFrameQueue;
	};
}
