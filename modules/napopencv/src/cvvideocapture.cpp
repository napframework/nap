#include "cvvideocapture.h"

// nap::cvvideocapture run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::CVVideoCapture)
	RTTI_PROPERTY("Backend",		&nap::CVVideoCapture::mAPIPreference,	nap::rtti::EPropertyMetaData::Default)
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


	void CVVideoCapture::stop()
	{
		onStop();
		if(mCaptureDevice.isOpened())
			mCaptureDevice.release();
	}


	int CVVideoCapture::getWidth() const
	{
		return static_cast<int>(getProperty(cv::CAP_PROP_FRAME_WIDTH));
	}


	int CVVideoCapture::getHeight() const
	{
		return static_cast<int>(getProperty(cv::CAP_PROP_FRAME_HEIGHT));
	}
}