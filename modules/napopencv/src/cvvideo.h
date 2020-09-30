/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "cvadapter.h"

// External Includes
#include <nap/resource.h>
#include <glm/glm.hpp>
#include <atomic>

namespace nap
{
	/**
	 * Captures frames from a video file or image sequence.
	 * The captured video frame is stored on the GPU when hardware acceleration is available (OpenCL).
	 * This is not a video player, only a non-blocking interface into an OpenCV video stream.
	 * If you need regular video playback, use the nap::Video object instead.
	 *
	 * Add this device to a nap::CVCaptureDevice to capture frames from the video stream, in a background thread.
	 * Note that this object should only be added once to a nap::CVCaptureDevice!
	 */
	class NAPAPI CVVideo : public CVAdapter
	{
		RTTI_ENABLE(CVAdapter)
	public:
		/**
		 * Initialize this object after de-serialization
		 * @param errorState contains the error message when initialization fails
		 * @return if initialization succeeded.
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * Loads a new video, the adapter is restarted.
		 * @param file new video file to load
		 * @return if the video loaded successfully. 
		 */
		bool changeVideo(const std::string& video, nap::utility::ErrorState& error);

		/**
		 * Returns capture frame width in pixels.
		 * @return capture frame width in pixels.
		 */
		int getWidth() const;

		/**
		 * Returns capture frame height in pixels.
		 * @return capture frame height in pixels.
		 */
		int getHeight() const;

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
		bool			mResize = false;						///< Property: 'Resize' if the frame is resized to the specified 'Size' after capture
		glm::ivec2		mSize = { 1280, 720 };					///< Property: 'Size' frame size, only used when 'Resize' is turned on.

	protected:
		
		virtual int getMatrixCount() override					{ return 1; }

		/**
		 * Called by the capture device. Opens the video file or image sequence.
		 * @param captureDevice device to open
		 * @param api api back-end to use
		 * @param error contains the error if the opening operation fails
		 */
		virtual bool onOpen(cv::VideoCapture& captureDevice, int api, nap::utility::ErrorState& error) override;

		/**
		 * This method decodes and returns the just grabbed frame.
		 * @param captureDevice the device to capture the frame from.
		 * @param outFrame contains the new decoded frame
		 * @return if decoding succeeded.
		 */
		virtual CVFrame onRetrieve(cv::VideoCapture& captureDevice, utility::ErrorState& error) override;

		/**
		 * Stores the current frame index
		 */
		virtual void onCopy() override;

		/**
		 * Occurs when the device is closed
		 */
		virtual void onClose() override;

	private:
		std::atomic<int>		mCurrentFrame   { 0 } ;				///< Last (Current) captured video frame index
		CVFrame					mCaptureFrame	{ 1 };
		CVFrame					mOutputFrame	{ 1 };
	};
}
