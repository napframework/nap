#include "cvadapter.h"
#include "cvcapturedevice.h"

// nap::cvadapter run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::CVAdapter)
	RTTI_PROPERTY("Backend", &nap::CVAdapter::mAPIPreference, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	void CVAdapter::setProperty(cv::VideoCaptureProperties propID, double value)
	{
		assert(mParent != nullptr);
		mParent->setProperty(*this, propID, value);
	}


	double CVAdapter::getProperty(cv::VideoCaptureProperties propID) const
	{
		return mCaptureDevice.get(propID);
	}


	bool CVAdapter::start(utility::ErrorState& errorState)
	{
		assert(!mCaptureDevice.isOpened());
		return this->onOpen(mCaptureDevice, static_cast<int>(mAPIPreference), errorState);
	}


	void CVAdapter::stop()
	{
		assert(mCaptureDevice.isOpened());
		mCaptureDevice.release();
		onClose();
	}


	cv::VideoCapture& CVAdapter::getCaptureDevice()
	{
		return mCaptureDevice;
	}


	const cv::VideoCapture& CVAdapter::getCaptureDevice() const
	{
		return mCaptureDevice;
	}


	nap::CVCaptureDevice& CVAdapter::getParent() const
	{
		assert(mParent != nullptr);
		return *mParent;
	}


	CVFrame CVAdapter::retrieve(utility::ErrorState& error)
	{
		return this->onRetrieve(mCaptureDevice, error);
	}

}