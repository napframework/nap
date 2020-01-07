#include "cvvideoadapter.h"

#include <nap/logger.h>
#include <mathutils.h>

// nap::cvvideoadapter run time class definition 
RTTI_BEGIN_CLASS(nap::CVVideoAdapter)
	RTTI_PROPERTY("ConvertRGB",		&nap::CVVideoAdapter::mConvertRGB,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FlipHorizontal",	&nap::CVVideoAdapter::mFlipHorizontal,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FlipVertical",	&nap::CVVideoAdapter::mFlipVertical,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Resize",			&nap::CVVideoAdapter::mResize,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Size",			&nap::CVVideoAdapter::mSize,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("File",			&nap::CVVideoAdapter::mFile,			nap::rtti::EPropertyMetaData::Required | nap::rtti::EPropertyMetaData::FileLink)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	bool CVVideoAdapter::init(utility::ErrorState& errorState)
	{
		return CVAdapter::init(errorState);
	}


	float CVVideoAdapter::getFramerate() const
	{
		return static_cast<float>(getProperty(cv::CAP_PROP_FPS));
	}


	float CVVideoAdapter::getLength()
	{
		return static_cast<float>(geFrameCount()) / getFramerate();
	}


	int CVVideoAdapter::geFrameCount() const
	{
		return static_cast<int>(getProperty(cv::VideoCaptureProperties::CAP_PROP_FRAME_COUNT));
	}


	void CVVideoAdapter::reset()
	{
		setFrame(0);
	}


	void CVVideoAdapter::setFrame(int frame)
	{
		// Clamp to range and set as property
		int req_frame = nap::math::clamp<int>(frame, 0, geFrameCount() - 1);
		nap::Logger::info("requesting frame: %d", req_frame);
		setProperty(cv::CAP_PROP_POS_FRAMES, static_cast<double>(req_frame));
	}


	int CVVideoAdapter::getFrame()
	{
		return mCurrentFrame;
	}


	void CVVideoAdapter::setTime(float time)
	{
		setFrame(static_cast<int>(time * getFramerate()));
	}


	float CVVideoAdapter::getTime()
	{
		return static_cast<float>(mCurrentFrame) / getFramerate();
	}


	bool CVVideoAdapter::onOpen(cv::VideoCapture& captureDevice, int api, nap::utility::ErrorState& error)
	{
		if (!error.check(captureDevice.open(mFile, api), "unable to open video file: %s", mFile.c_str()))
			return false;
		mCurrentFrame = 0;
		return true;
	}


	bool CVVideoAdapter::onRetrieve(cv::VideoCapture& captureDevice, CVFrame& outFrame, utility::ErrorState& error)
	{
		assert(outFrame.getCount() >= 1);
		if (!captureDevice.retrieve(mFrame[0]))
		{
			error.fail("%s: no new frame available", mID.c_str());
			return false;
		}

		// Resize frame if required
		// Otherwise simply copy mat reference (no actual data copy takes place)
		if (mResize)
			cv::resize(mFrame[0], outFrame[0], cv::Size(mSize.x, mSize.y));
		else
			outFrame[0] = mFrame[0];

		// Convert to RGB
		if (mConvertRGB)
			cv::cvtColor(outFrame[0], outFrame[0], cv::COLOR_BGR2RGB);

		// Flip horizontal
		if (mFlipHorizontal)
			cv::flip(outFrame[0], outFrame[0], 1);

		// Flip vertical
		if (mFlipVertical)
			cv::flip(outFrame[0], outFrame[0], 0);

		return true;
	}


	void CVVideoAdapter::onCopy()
	{
		mCurrentFrame = getProperty(cv::VideoCaptureProperties::CAP_PROP_POS_FRAMES) - 1;
		nap::Logger::info("set frame: %d", static_cast<int>(mCurrentFrame));
	}
}