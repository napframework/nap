#include "flexdevice.h"

// nap::flexdevice run time class definition 
RTTI_BEGIN_CLASS(nap::FlexDevice)
	// Put additional properties here
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