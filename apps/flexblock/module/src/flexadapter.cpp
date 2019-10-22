#include "flexadapter.h"

// nap::flexadapter run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::FlexAdapter)
	RTTI_PROPERTY("Enabled",	&nap::FlexAdapter::mEnabled,	nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	FlexAdapter::~FlexAdapter()			{ }


	bool FlexAdapter::init(utility::ErrorState& errorState)
	{
		return true;
	}


	void FlexAdapter::compute(const FlexDevice& device)
	{
		if(mEnabled)
			onCompute(device);
	}
}