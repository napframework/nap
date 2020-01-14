#include "cvvideo.h"
#include "cvcapturedevice.h"

#include <nap/logger.h>
#include <mathutils.h>

// nap::cvvideoadapter run time class definition 
RTTI_BEGIN_CLASS(nap::CVVideo)
	RTTI_PROPERTY("ConvertRGB",		&nap::CVVideo::mConvertRGB,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FlipHorizontal",	&nap::CVVideo::mFlipHorizontal,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FlipVertical",	&nap::CVVideo::mFlipVertical,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Resize",			&nap::CVVideo::mResize,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Size",			&nap::CVVideo::mSize,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("File",			&nap::CVVideo::mFile,			nap::rtti::EPropertyMetaData::Required | nap::rtti::EPropertyMetaData::FileLink)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	bool CVVideo::init(utility::ErrorState& errorState)
	{
		return CVAdapter::init(errorState);
	}


	bool CVVideo::onOpen(cv::VideoCapture& captureDevice, int api, nap::utility::ErrorState& error)
	{
		if (!error.check(captureDevice.open(mFile, api), "unable to open video file: %s", mFile.c_str()))
			return false;

		mCurrentFrame = 0;
		return true;
	}


	bool CVVideo::changeVideo(const std::string& video, nap::utility::ErrorState& error)
	{
		CVCaptureDevice& capture_device = getParent();
		capture_device.stop();
		mFile = video;
		return capture_device.start(error);
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


	CVFrame CVVideo::onRetrieve(cv::VideoCapture& captureDevice, utility::ErrorState& error)
	{
		if (!captureDevice.retrieve(mCaptureFrame[0]))
		{
			error.fail("%s: no new frame available", mID.c_str());
			return CVFrame();
		}

		// Resize frame if required
		// Otherwise simply copy mat reference (no actual data copy takes place)
		if (mResize && (mCaptureFrame[0].cols != mSize.x || mCaptureFrame[0].rows != mSize.y))
		{
			cv::resize(mCaptureFrame[0], mOutputFrame[0], cv::Size(mSize.x, mSize.y));
		}
		else
		{
			mOutputFrame[0] = mCaptureFrame[0];
		}

		// Convert to RGB
		if (mConvertRGB)
			cv::cvtColor(mOutputFrame[0], mOutputFrame[0], cv::COLOR_BGR2RGB);

		// Flip horizontal
		if (mFlipHorizontal)
			cv::flip(mOutputFrame[0], mOutputFrame[0], 1);

		// Flip vertical
		if (mFlipVertical)
			cv::flip(mOutputFrame[0], mOutputFrame[0], 0);

		return mOutputFrame;
	}


	void CVVideo::onCopy()
	{
		mCurrentFrame = getProperty(cv::VideoCaptureProperties::CAP_PROP_POS_FRAMES) - 1;
		nap::Logger::info("set frame: %d", static_cast<int>(mCurrentFrame));
	}
}