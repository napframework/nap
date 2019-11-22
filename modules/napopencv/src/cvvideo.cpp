#include "cvvideo.h"

// External Includes
#include <utility/fileutils.h>
#include <nap/timer.h>

RTTI_BEGIN_ENUM(nap::ECVPlaybackMode)
	RTTI_ENUM_VALUE(nap::ECVPlaybackMode::Manual,		"Manual"),
	RTTI_ENUM_VALUE(nap::ECVPlaybackMode::Automatic,	"Automatic")
RTTI_END_ENUM


// nap::cvvideo run time class definition 
RTTI_BEGIN_CLASS(nap::CVVideo)
	RTTI_PROPERTY("OverrideFPS",	&nap::CVVideo::mOverrideFPS,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FPS",			&nap::CVVideo::mFramerate,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Playback",		&nap::CVVideo::mMode,			nap::rtti::EPropertyMetaData::Default)
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
		return mFramerate;
	}


	void CVVideo::nextFrame()
	{
		{
			std::lock_guard<std::mutex> lock(mCaptureMutex);
			mNextFrame = true;
		}
		mCaptureCondition.notify_one();
	}


	void CVVideo::play()
	{
		{
			std::lock_guard<std::mutex> lock(mCaptureMutex);
			mPlay = true;
		}
		mCaptureCondition.notify_one();
	}


	void CVVideo::pause()
	{
		{
			std::lock_guard<std::mutex> lock(mCaptureMutex);
			mPlay = false;
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
		// Set framerate
		mFramerate = mOverrideFPS ? mFramerate : static_cast<float>(getProperty(cv::CAP_PROP_FPS));

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

		while (true)
		{
			// Wait for playback to be enabled, a new frame request is issued or request to stop is made
			// Exit loop immediately when a stop is requested. Otherwise process next frame
			{
				std::unique_lock<std::mutex> lock(mCaptureMutex);
				mCaptureCondition.wait(lock, [this]()
				{
					return (mStop || mPlay || mNextFrame);
				});

				// Exit loop when exit has been triggered
				if (mStop)
				{
					break;
				}
			}

			// Get next frame if playback is disabled
			if (!mPlay && mNextFrame)
			{
				getCaptureDevice() >> cap_frame;
				if (!cap_frame.empty())
				{
					{
						std::lock_guard<std::mutex> lock(mCaptureMutex);
						cap_frame.copyTo(mCaptureMat);
					}
					mFrameAvailable = true;
				}
			}

			mNextFrame = false;
		}
	}
}