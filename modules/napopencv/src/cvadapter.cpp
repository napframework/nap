#include "cvadapter.h"
#include "cvcapturedevice.h"

// nap::cvadapter run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::CVAdapter)
	RTTI_PROPERTY("Backend", &nap::CVAdapter::mAPIPreference, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	bool CVAdapter::init(utility::ErrorState& errorState)
	{
		mCaptureDevice.setExceptionMode(false);
		return true;
	}


	void CVAdapter::setProperty(cv::VideoCaptureProperties propID, double value)
	{
		if (isOpen())
		{
			assert(mParent != nullptr);
			mParent->setProperty(*this, propID, value);
		}
	}


	double CVAdapter::getProperty(cv::VideoCaptureProperties propID) const
	{
		return isOpen() ? mCaptureDevice.get(propID) : -1.0;
	}


	nap::CVCaptureErrorMap CVAdapter::getErrors() const
	{
		assert(mParent != nullptr);
		return mParent->getErrors(*this);
	}


	bool CVAdapter::hasErrors(CVCaptureErrorMap& outErrors)
	{
		// First check if there's a global error
		assert(mParent != nullptr);
		if (!mParent->hasErrors())
			return false;

		// Now check if there's an adapter specific error
		return mParent->getErrors(*this).empty();
	}


	bool CVAdapter::isOpen() const
	{
		return mCaptureDevice.isOpened();
	}


	bool CVAdapter::open(utility::ErrorState& errorState)
	{
		assert(!mCaptureDevice.isOpened());
		return onOpen(mCaptureDevice, static_cast<int>(mAPIPreference), errorState);
	}


	void CVAdapter::close()
	{
		assert(mCaptureDevice.isOpened());
		mCaptureDevice.release();
		onClose();
	}


	bool CVAdapter::restart(utility::ErrorState& error)
	{
		assert(mParent != nullptr);
		return mParent->restart(*this, error);
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