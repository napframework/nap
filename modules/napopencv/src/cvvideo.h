#pragma once

// Local Includes
#include "cvvideocapture.h"

// External Includes
#include <thread>
#include <future>

namespace nap
{
	/**
	 * OpenCV Video playback mode
	 */
	enum class ECVPlaybackMode : int
	{
		Manual		= 0,	///< Manual playback mode
		Automatic	= 1		///< Automatic playback mode
	};


	/**
	 * Plays back a video file or image sequence from disk using OpenCV.
	 * Playback can be set to manual or automatic. 
	 * Manual playback requires explicit frame requests.
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
		 * @return currently used playback speed in FPS
		 */
		float getFramerate() const;

		/**
		 * Go to next frame
		 */
		void nextFrame();

		/**
		 * Start playback
		 */
		void play();

		/**
		 * Pause playback
		 */
		void pause();

		ECVPlaybackMode mMode = ECVPlaybackMode::Automatic;		///< Property: 'Playback' video playback mode, manual or automatic.
		std::string		mFile;									///< Property: 'File' the video file or image sequence. Sequences should be formatted as "my_seq.%02d.png
		bool			mOverrideFPS = false;					///< Property: 'OverrideFPS'  when set to true the 'FPS' value is used, otherwise the default video playback speed.
		float			mFramerate = 25;						///< Property: 'FPS' video playback speed in frames per second


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
		std::future<void>		mCaptureTask;				///< The thread that monitor the read thread
		std::mutex				mCaptureMutex;				///< The mutex that safe guards the capture thread
		std::condition_variable	mCaptureCondition;			///< Used for telling the polling task to continue
		bool					mStop = false;				///< Signals the capture thread to stop capturing video
		bool					mPlay = false;				///< Playback video
		bool					mNextFrame = false;			///< Proceed to next frame

		cv::UMat				mCaptureMat;				///< The GPU / CPU matrix that holds the most recent captured video frame
		std::atomic<bool>		mFrameAvailable = false;	///< If a new frame is captured

		/**
		 * Captures new frames.
		 */
		void capture();
	};
}
