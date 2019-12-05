#include "cvvideo.h"

// External Includes
#include <utility/fileutils.h>
#include <nap/timer.h>
#include <mathutils.h>
#include <nap/logger.h>


// nap::cvvideo run time class definition 
RTTI_BEGIN_CLASS(nap::CVVideo)
	RTTI_PROPERTY("File",			&nap::CVVideo::mFile,			nap::rtti::EPropertyMetaData::Required | nap::rtti::EPropertyMetaData::FileLink)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	CVVideo::~CVVideo()			{ }


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


	void CVVideo::reset()
	{
		setFrame(0);
	}


	void CVVideo::setFrame(int frame)
	{
		// Clamp to range and set as property
		int req_frame = nap::math::clamp<int>(frame, 0, geFrameCount() - 1);
		nap::Logger::info("requesting frame: %d", req_frame);
		setProperty(cv::CAP_PROP_POS_FRAMES, static_cast<double>(req_frame));
	}


	int CVVideo::getFrame()
	{
		return mCurrentFrame;
	}


	void CVVideo::setTime(float time)
	{	
		setFrame(static_cast<int>(time * getFramerate()));
	}


	float CVVideo::getTime()
	{
		return static_cast<float>(mCurrentFrame) / getFramerate();
	}


	bool CVVideo::onOpen(cv::VideoCapture& captureDevice, int api, nap::utility::ErrorState& error)
	{
		if (!error.check(captureDevice.open(mFile, api), "unable to open video file: %s", mFile.c_str()))
			return false;
		return true;
	}


	bool CVVideo::onRetrieve(cv::VideoCapture& captureDevice, cv::UMat& outFrame, utility::ErrorState& error)
	{
		if (!captureDevice.retrieve(outFrame))
		{
			error.fail("%s: no new frame available", mID.c_str());
			return false;
		}
		return true;
	}


	void CVVideo::onCopy()
	{
		mCurrentFrame = getProperty(cv::VideoCaptureProperties::CAP_PROP_POS_FRAMES) - 1;
		nap::Logger::info("set frame: %d", static_cast<int>(mCurrentFrame));
	}
}