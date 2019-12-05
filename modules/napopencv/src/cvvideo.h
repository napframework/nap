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
		void reset();

		/**
		 * Selects the frame to be captured next, where 0 is the first frame.
		 * The requested frame is queued immediately for capture.
		 * Note that only the last request is considered when multiple requests are made before the frame is available.
		 * This function clamps the frame when out of bounds.
		 * @param frame the requested frame inside the stream
		 * @return if operation succeeded
		 */
		void setFrame(int frame);

		/**
		 * Returns the current frame index.
		 * This value is updated when a new frame is captured.
		 * @return current position of the marker inside the video stream.
		 */
		int getFrame();

		/**
		 * Set the marker to a specific location inside the video stream.
		 * The requested frame at the given time is captured immediately in the background.
		 * Note that only the last request is considered when multiple requests are made before the frame is available.
		 * This function clamps the time when out of bounds.
		 * @param time time in seconds
		 * @return if operation succeeded
		 */
		void setTime(float time);

		/**
		 * @return the current time in seconds of the marker in the video stream
		 */
		float getTime();

		std::string		mFile;									///< Property: 'File' the video file or image sequence. Sequences should be formatted as "my_seq.%02d.png

	protected:
		/**
		 * Called by the capture device. Opens the video file or image sequence.
		 * @param captureDevice device to open
		 * @param api api back-end to use
		 * @param error contains the error if the opening operation fails
		 */
		virtual bool onOpen(cv::VideoCapture& captureDevice, int api, nap::utility::ErrorState& error) override;
		
		/**
		 * This method decodes and returns the just grabbed frame.
		 * Needs to be implemented in a derived class.
		 * Return false when decoding fails, otherwise return true.
		 * The 'outFrame' should contain the decoded frame on success.
		 * @param captureDevice the device to capture the frame from.
		 * @param outFrame contains the new decoded frame
		 * @return if decoding succeeded.
		 */
		virtual bool onRetrieve(cv::VideoCapture& captureDevice, cv::UMat& outFrame, utility::ErrorState& error) override;

		/**
		 *	Stores the current frame index
		 */
		virtual void onCopy() override;

	private:
		bool					mSetFrameMarker = false;		///< If a new frame location needs to be set
		int						mMarkerFrame = 0;				///< Manual set location of marker
		std::atomic<int>		mCurrentFrame = 0;				///< Last (Current) captured video frame index
	};
}
