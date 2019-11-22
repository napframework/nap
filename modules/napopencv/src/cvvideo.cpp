#include "cvvideo.h"

// External Includes
#include <utility/fileutils.h>
#include <nap/timer.h>
#include <mathutils.h>


// nap::cvvideo run time class definition 
RTTI_BEGIN_CLASS(nap::CVVideo)
	RTTI_PROPERTY("File",			&nap::CVVideo::mFile,			nap::rtti::EPropertyMetaData::Required | nap::rtti::EPropertyMetaData::FileLink)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	CVVideo::~CVVideo()			{ }


	bool CVVideo::grab(cv::UMat& target)
	{
		// Check if a new frame is available
		if (!mFrameAvailable)
			return false;

		// Copy last captured frame using a deep copy.
		// Again, the deep copy is necessary because a weak copy allows
		// for the data to be updated by the capture loop whilst still processing on another thread.
		std::lock_guard<std::mutex> lock(mCaptureMutex);
		mCaptureMat.copyTo(target);
		mFrameAvailable = false;
		return true;
	}


	float CVVideo::getFramerate() const
	{
		return static_cast<float>(getProperty(cv::CAP_PROP_FPS));
	}


	float CVVideo::getLength()
	{
		return static_cast<float>(geFrameCount()) / getFramerate();
	}


	int CVVideo::geFrameCount() const
	{
		return static_cast<int>(getProperty(cv::VideoCaptureProperties::CAP_PROP_FRAME_COUNT));
	}


	bool CVVideo::reset()
	{
		if (setProperty(cv::VideoCaptureProperties::CAP_PROP_POS_FRAMES, (double)0))
		{
			captureNextFrame();
			return true;
		}
		return false;
	}


	void CVVideo::setFrame(int frame)
	{
		captureFrame(frame);
	}


	int CVVideo::getFrame()
	{
		return static_cast<int>(getProperty(cv::CAP_PROP_POS_FRAMES));
	}


	void CVVideo::setTime(float time)
	{	
		setFrame(static_cast<int>(time * getFramerate()));
	}


	float CVVideo::getTime()
	{
		return static_cast<float>(mCurrentFrame) / getFramerate();
	}


	void CVVideo::captureNextFrame()
	{
		{
			std::lock_guard<std::mutex> lock(mCaptureMutex);
			mCaptureFrame = true;
		}
		mCaptureCondition.notify_one();
	}


	void CVVideo::captureFrame(int frame)
	{
		// Clamp to range
		int req_frame = nap::math::clamp<int>(frame, 0, geFrameCount() - 1);
		
		// Acquire lock, setup variables and try to wake up capture thread
		{
			std::unique_lock<std::mutex> lock(mCaptureMutex);
			mSetFrameMarker = true;
			mCurrentFrame	= frame;
			mCaptureFrame	= true;
		}
		mCaptureCondition.notify_one();
	}


	bool CVVideo::onOpen(cv::VideoCapture& captureDevice, int api, nap::utility::ErrorState& error)
	{
		if (!error.check(captureDevice.open(mFile, api), "unable to open video file: %s", mFile.c_str()))
			return false;
		return true;
	}


	bool CVVideo::onStart(cv::VideoCapture& captureDevice, utility::ErrorState& error)
	{
		mStop = false;
		mCaptureTask = std::async(std::launch::async, std::bind(&CVVideo::capture, this));
		return true;
	}


	void CVVideo::onStop()
	{
		// Stop capturing thread and notify worker
		{
			std::lock_guard<std::mutex> lock(mCaptureMutex);
			mStop = true;
		}
		mCaptureCondition.notify_one();

		// Wait for the thread to complete
		if (mCaptureTask.valid())
			mCaptureTask.wait();
	}


	void CVVideo::capture()
	{
		// Some global loop variables
		nap::SystemTimer timer;
		cv::UMat cap_frame;

		bool update_marker = false;
		float marker_location = 0.0f;

		while (true)
		{
			// Wait for playback to be enabled, a new frame request is issued or request to stop is made
			// Exit loop immediately when a stop is requested. Otherwise process next frame
			{
				std::unique_lock<std::mutex> lock(mCaptureMutex);
				mCaptureCondition.wait(lock, [this]()
				{
					return (mStop || mCaptureFrame);
				});

				// Exit loop when exit has been triggered
				if (mStop)
				{
					break;
				}

				// Copy some processing variables inside lock
				update_marker	= mSetFrameMarker;
				marker_location = mCurrentFrame;
				
				// Reset input
				mSetFrameMarker	= false;
				mCaptureFrame	= false;
			}

			// Update marker in video stream if requested
			if (update_marker)
				setProperty(cv::VideoCaptureProperties::CAP_PROP_POS_FRAMES, marker_location);

			// Grab next frame
			getCaptureDevice() >> cap_frame;

			// Update current frame if we're not manually updating the marker
			if(!update_marker)
				mCurrentFrame = static_cast<int>(getProperty(cv::VideoCaptureProperties::CAP_PROP_POS_FRAMES));

			// If no next frame is available, it's most likely the end of the stream
			if (cap_frame.empty())
				return;

			// Process
			{
				std::lock_guard<std::mutex> lock(mCaptureMutex);
				cap_frame.copyTo(mCaptureMat);
			}
			mFrameAvailable = true;
		}
	}
}