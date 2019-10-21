#include "flexadapter.h"

// nap::flexadapter run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::FlexAdapter)
	RTTI_PROPERTY("Enabled",	&nap::FlexAdapter::mEnabled,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FlexDevice", &nap::FlexAdapter::mFlexDevice, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	FlexAdapter::~FlexAdapter()			{ }


	bool FlexAdapter::init(utility::ErrorState& errorState)
	{
		return true;
	}


	void FlexAdapter::compute()
	{
		if(mEnabled)
			onCompute(*mFlexDevice);
	}
}