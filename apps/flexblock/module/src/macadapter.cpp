#include "macadapter.h"

// nap::macadapter run time class definition 
RTTI_BEGIN_CLASS(nap::MACAdapter)
	RTTI_PROPERTY("Set Digital Pin",		&nap::MACAdapter::mSetDigitalPin,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Motor Steps Per Meter",	&nap::MACAdapter::mMotorStepsPerMeter,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Motor Step Offset",		&nap::MACAdapter::mMotorStepOffset,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Motor Mapping",			&nap::MACAdapter::mMotorMapping,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Controller",				&nap::MACAdapter::mController,			nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	MACAdapter::~MACAdapter()			{ }


	bool MACAdapter::init(utility::ErrorState& errorState)
	{
		return true;
	}


	void MACAdapter::onCompute(const FlexDevice& device)
	{
		std::vector<float> lengths;
		device.getRopeLengths(lengths);
	}
}