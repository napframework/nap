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
	}


	void CVAdapter::setProperty(cv::VideoCaptureProperties propID, double value)
	{
		assert(mParent != nullptr);
		mParent->setProperty(*this, propID, value);
	}


	double CVAdapter::getProperty(cv::VideoCaptureProperties propID) const
	{
		return mCaptureDevice.get(propID);
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

	bool CVAdapter::start(utility::ErrorState& errorState)
	{
		mStarted = false;
		assert(!mCaptureDevice.isOpened());
		mStarted = onOpen(mCaptureDevice, static_cast<int>(mAPIPreference), errorState);
		return mStarted;
	}


	void CVAdapter::stop()
	{
		assert(mCaptureDevice.isOpened());
		mCaptureDevice.release();
		mStarted = false;
		onClose();
	}


	bool CVAdapter::restart(utility::ErrorState& error)
	{
		// Stop capture device
		CVCaptureDevice& capture_device = getParent();
		capture_device.stop();

		// Stop this device
		if (started())
			this->stop();

		// Start the device
		bool opened = start(error);

		// Start capturing again
		return (opened && capture_device.start(error));
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