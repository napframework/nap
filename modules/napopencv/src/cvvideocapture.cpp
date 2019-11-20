#include "cvvideocapture.h"

// nap::cvvideocapture run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::CVVideoCapture)
	RTTI_PROPERTY("Backend",		&nap::CVVideoCapture::mAPIPreference,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FrameWidth",		&nap::CVVideoCapture::mFrameWidth,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FrameHeight",	&nap::CVVideoCapture::mFrameHeight,		nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	CVVideoCapture::~CVVideoCapture()			{ }


	bool CVVideoCapture::start(utility::ErrorState& errorState)
	{
		assert(!mCaptureDevice.isOpened());
		if (!onOpen(mCaptureDevice, static_cast<int>(mAPIPreference), errorState))
			return false;
		
		// Set capture dimensions
		if (!mCaptureDevice.set(cv::CAP_PROP_FRAME_WIDTH, (double)mFrameWidth))
		{
			errorState.fail("unable to set video capture frame width to: %d", mFrameWidth);
			return false;
		}

		if (!mCaptureDevice.set(cv::CAP_PROP_FRAME_HEIGHT, (double)mFrameHeight))
		{
			errorState.fail("unable to set video capture frame height to: %d", mFrameHeight);
			return false;
		}

		return onStart(mCaptureDevice, errorState);
	}


	bool CVVideoCapture::setProperty(cv::VideoCaptureProperties propID, double value)
	{
		return mCaptureDevice.set(propID, value);
	}


	double CVVideoCapture::getProperty(cv::VideoCaptureProperties propID) const
	{
		return mCaptureDevice.get(propID);
	}


	int CVVideoCapture::getWidth() const
	{
		return static_cast<int>(getProperty(cv::CAP_PROP_FRAME_WIDTH));
	}


	int CVVideoCapture::getHeight() const
	{
		return static_cast<int>(getProperty(cv::CAP_PROP_FRAME_HEIGHT));
	}


	bool CVVideoCapture::setWidth(int width)
	{
		return setProperty(cv::CAP_PROP_FRAME_WIDTH, static_cast<double>(width));
	}


	bool CVVideoCapture::setHeight(int height)
	{
		return setProperty(cv::CAP_PROP_FRAME_HEIGHT, static_cast<double>(height));
	}


	void CVVideoCapture::stop()
	{
		onStop();
		if(mCaptureDevice.isOpened())
			mCaptureDevice.release();
	}
}