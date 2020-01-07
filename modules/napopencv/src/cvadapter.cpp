#include "cvadapter.h"
#include "cvvideocapture.h"

// nap::cvadapter run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::CVAdapter)
	RTTI_PROPERTY("Backend", &nap::CVAdapter::mAPIPreference, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	CVAdapter::~CVAdapter()			{ }


	bool CVAdapter::init(utility::ErrorState& errorState)
	{
		return true;
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


	bool CVAdapter::open(nap::utility::ErrorState& error)
	{
		return this->onOpen(getCaptureDevice(), static_cast<int>(mAPIPreference), error);
	}


	bool CVAdapter::retrieve(CVFrame& frame, utility::ErrorState& error)
	{
		return this->onRetrieve(getCaptureDevice(), frame, error);
	}

}