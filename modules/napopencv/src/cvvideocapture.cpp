#include "cvvideocapture.h"

// nap::cvvideocapture run time class definition 
RTTI_BEGIN_CLASS(nap::CVVideoCapture)
	RTTI_PROPERTY("DeviceIndex", &nap::CVVideoCapture::mDeviceIndex, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	CVVideoCapture::~CVVideoCapture()			{ }


	bool CVVideoCapture::init(utility::ErrorState& errorState)
	{
		return true;
	}


	bool CVVideoCapture::start(utility::ErrorState& errorState)
	{
		assert(!mCaptureDevice.isOpened());
		if(!errorState.check(mCaptureDevice.open(static_cast<int>(mDeviceIndex)), "unable to open video capture device: %s", mDeviceIndex))
			return false;

		mStopCapturing = false;
		mCaptureTask = std::async(std::launch::async, std::bind(&CVVideoCapture::capture, this));
		return true;
	}


	void CVVideoCapture::stop()
	{
		// Stop capturing thread
		mStopCapturing = true;
		if (mCaptureTask.valid())
			mCaptureTask.wait();

		// Close device
		assert(mCaptureDevice.isOpened());
		mCaptureDevice.release();
	}


	void CVVideoCapture::copy(cv::UMat& target)
	{
		mCaptureMat.copyTo(target);
		mNewFrame = false;
	}


	bool CVVideoCapture::hasNewFrame()
	{
		return mNewFrame;
	}


	void CVVideoCapture::capture()
	{
		while (!mStopCapturing)
		{
			mCaptureDevice >> mCaptureMat;
			mNewFrame = true;
		}
	}
}