#pragma once

// Local Includes
#include "cvvideocapture.h"

// External Includes
#include <thread>
#include <future>

namespace nap
{
	/**
	 * OpenCV video capture device. 
	 * Reads video files and is able to capture frames from the loaded video stream.
	 * This is not a video player, only a non-blocking interface into an OpenCV video stream.
	 * Call captureFrame(int) or captureNextFrame() to capture a frame in the background.
	 */
	class NAPAPI CVVideo : public CVVideoCapture
	{
		RTTI_ENABLE(CVVideoCapture)
	public:

		// Stops the device
		virtual ~CVVideo() override;

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
		 * @return intended video playback speed in frames per second
		 */
		float getFramerate() const;

		/**
		 * @return length of the video in seconds based on framerate and frame count.
		 */
		float getLength();

		/**
		 * @return number of frames in video file or sequence
		 */
		int geFrameCount() const;

		/**
		 * Reset the video to the beginning of the video stream.
		 * The first frame is captured immediately in the background.
		 */
		bool reset();

		/**
		 * Set the marker to a specific frame inside the video stream.
		 * This function clamps the frame when out of bounds.
		 * The requested frame is captured immediately in the background.
		 * @param frame the requested frame inside the stream
		 * @return if operation succeeded
		 */
		void setFrame(int frame);

		/**
		 * @return current position of the marker inside the video stream
		 */
		int getFrame();

		/**
		 * Set the marker to a specific location inside the video stream.
		 * This function clamps the time when out of bounds.
		 * The requested frame at the given time is captured immediately in the background.
		 * @param time time in seconds
		 * @return if operation succeeded
		 */
		void setTime(float time);

		/**
		 * @return the current time in seconds of the marker in the video stream
		 */
		float getTime();

		/**
		 * Tells the capture thread to capture the next available frame.
		 * This is a non-blocking call!
		 */
		void captureNextFrame();

		/**
		 * Tells the capture thread to capture the given frame.
		 * This is a non-blocking call!
		 * @param frame index of the frame to capture
		 */
		void captureFrame(int frame);

		std::string		mFile;									///< Property: 'File' the video file or image sequence. Sequences should be formatted as "my_seq.%02d.png

	protected:
		/**
		 * Called by the capture device. Opens the video file or image sequence
		 */
		virtual bool onOpen(cv::VideoCapture& captureDevice, int api, nap::utility::ErrorState& error) override;

		/**
		 * Called by the capture device. Starts video playback
		 */
		virtual bool onStart(cv::VideoCapture& captureDevice, utility::ErrorState& error) override;

		/**
		 * Called by the capture device. Stops video playback
		 */
		virtual void onStop() override;

	private:
		std::future<void>		mCaptureTask;					///< The thread that monitor the read thread
		std::mutex				mCaptureMutex;					///< The mutex that safe guards the capture thread
		std::condition_variable	mCaptureCondition;				///< Used for telling the polling task to continue
		bool					mStop = false;					///< Signals the capture thread to stop capturing video

		cv::UMat				mCaptureMat;					///< The GPU / CPU matrix that holds the most recent captured video frame
		bool					mCaptureFrame = true ;			///< Proceed to next frame
		std::atomic<bool>		mFrameAvailable = { false };	///< If a new frame is captured
		bool					mSetFrameMarker = false;		///< If a new frame location needs to be set
		int						mCurrentFrame = 0;				///< Current frame location of marker

		/**
		 * Captures new frames.
		 */
		void capture();
	};
}
