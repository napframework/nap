#include "cvwebstream.h"
#include "cvcapturedevice.h"

#include <nap/logger.h>
#include <mathutils.h>

// nap::cvvideoadapter run time class definition 
RTTI_BEGIN_CLASS(nap::CVWebStream)
	RTTI_PROPERTY("ConvertRGB",		&nap::CVWebStream::mConvertRGB,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FlipHorizontal",	&nap::CVWebStream::mFlipHorizontal,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FlipVertical",	&nap::CVWebStream::mFlipVertical,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Resize",			&nap::CVWebStream::mResize,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Size",			&nap::CVWebStream::mSize,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Link",			&nap::CVWebStream::mLink,			nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	bool CVWebStream::init(utility::ErrorState& errorState)
	{
		return CVAdapter::init(errorState);
	}


	int CVWebStream::getWidth() const
	{
		return static_cast<int>(getProperty(cv::CAP_PROP_FRAME_WIDTH));
	}


	int CVWebStream::getHeight() const
	{
		return static_cast<int>(getProperty(cv::CAP_PROP_FRAME_HEIGHT));
	}


	bool CVWebStream::onOpen(cv::VideoCapture& captureDevice, int api, nap::utility::ErrorState& error)
	{
		if (!error.check(captureDevice.open(mLink, api), "Unable to open network stream: %s", mLink.c_str()))
			return false;

		// Reset some internally managed variables
		mCaptureFrame	= CVFrame(1, this);
		mOutputFrame	= CVFrame(1, this);

		return true;
	}


	CVFrame CVWebStream::onRetrieve(cv::VideoCapture& captureDevice, utility::ErrorState& error)
	{
		// Retrieve currently stored frame
		if (!captureDevice.retrieve(mCaptureFrame[0]))
		{
			error.fail("%s: No new frame available", mID.c_str());
			return CVFrame();
		}

		// Resize frame if required
		// Otherwise simply copy mat reference (no actual data copy takes place)
		if (mResize && (mCaptureFrame[0].cols != mSize.x || mCaptureFrame[0].rows != mSize.y))
			cv::resize(mCaptureFrame[0], mOutputFrame[0], cv::Size(mSize.x, mSize.y));
		else
			mOutputFrame[0] = mCaptureFrame[0];

		// Convert to RGB
		if (mConvertRGB)
			cv::cvtColor(mOutputFrame[0], mOutputFrame[0], cv::COLOR_BGR2RGB);

		// Flip horizontal
		if (mFlipHorizontal)
			cv::flip(mOutputFrame[0], mOutputFrame[0], 1);

		// Flip vertical
		if (mFlipVertical)
			cv::flip(mOutputFrame[0], mOutputFrame[0], 0);

		// Clone and return
		return mOutputFrame.clone();
	}
}