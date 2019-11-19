#include "cvvideocapture.h"

// nap::cvvideocapture run time class definition 
RTTI_BEGIN_CLASS(nap::CVVideoCapture)
	RTTI_PROPERTY("DeviceIndex",	&nap::CVVideoCapture::mDeviceIndex, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FrameWidth",		&nap::CVVideoCapture::mFrameWidth,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FrameHeight",	&nap::CVVideoCapture::mFrameHeight, nap::rtti::EPropertyMetaData::Default)
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
		if(!errorState.check(mCaptureDevice.open(static_cast<int>(mDeviceIndex)), "unable to open video capture device: %d", mDeviceIndex))
			return false;

		if (!mCaptureDevice.set(cv::CAP_PROP_FRAME_WIDTH, (double)mFrameWidth))
		{
			errorState.fail("unable to set video capture frame width: %d", mFrameWidth);
			return false;
		}

		if (!mCaptureDevice.set(cv::CAP_PROP_FRAME_HEIGHT, (double)mFrameHeight))
		{
			errorState.fail("unable to set video capture frame width: %d", mFrameWidth);
			return false;
		}

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


	bool CVVideoCapture::grab(cv::UMat& target)
	{
		std::lock_guard<std::mutex> lock(mCaptureMutex);
		if (!mNewFrame)
			return false;

		mCaptureMat.copyTo(target);
		mNewFrame = false;
		return true;
	}


	void CVVideoCapture::capture()
	{
		cv::UMat cap_frame;
		while (!mStopCapturing)
		{
			// Fetch frame
			mCaptureDevice >> cap_frame;
			if(cap_frame.empty())
				continue;

			// Flip and convert color
			cv::flip(cap_frame, cap_frame, 0);
			cv::cvtColor(cap_frame, cap_frame, cv::COLOR_BGR2RGB);
			
			// Copy
			std::lock_guard<std::mutex> lock(mCaptureMutex);
			cap_frame.copyTo(mCaptureMat);
			mNewFrame = true;
		}
	}
}