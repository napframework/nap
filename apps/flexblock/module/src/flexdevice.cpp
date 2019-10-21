#include "flexdevice.h"
#include "flexadapter.h"

// nap::flexdevice run time class definition 
RTTI_BEGIN_CLASS(nap::FlexDevice)
	RTTI_PROPERTY("FlexBlockShape",		&nap::FlexDevice::mFlexBlockShape,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Frequency",			&nap::FlexDevice::mUpdateFrequency,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Slack Minimum",		&nap::FlexDevice::mSlack,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Sinus Amplitude",	&nap::FlexDevice::mSinusAmplitude,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Sinus Frequency",	&nap::FlexDevice::mSinusFrequency,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Override Minimum",	&nap::FlexDevice::mOverrideMinimum,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Adapters",			&nap::FlexDevice::mAdapters,			nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	FlexDevice::~FlexDevice()			{ }


	bool FlexDevice::init(utility::ErrorState& errorState)
	{
		return true;
	}


	bool FlexDevice::start(utility::ErrorState& errorState)
	{
		return true;
	}


	void FlexDevice::stop()
	{

	}
}