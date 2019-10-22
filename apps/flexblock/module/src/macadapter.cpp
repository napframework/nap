#include "macadapter.h"
#include "flexdevice.h"

// nap::macadapter run time class definition 
RTTI_BEGIN_CLASS(nap::MACAdapter)
	RTTI_PROPERTY("Set Digital Pin",		&nap::MACAdapter::mSetDigitalPin,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Motor Steps Per Meter",	&nap::MACAdapter::mMotorStepsPerMeter,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Motor Step Offset",		&nap::MACAdapter::mMotorStepOffset,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Sinus Amplitude",		&nap::MACAdapter::mSinusAmplitude,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Sinus Frequency",		&nap::MACAdapter::mSinusFrequency,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Override Minimum",		&nap::MACAdapter::mOverrideMinimum,		nap::rtti::EPropertyMetaData::Default)
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


	void MACAdapter::setMotorOverrides(const std::vector<float>& inputs)
	{
		std::lock_guard<std::mutex> lock(mOverrideMutex);
		assert(mMotorOverrides.size() == inputs.size());
		mMotorOverrides = inputs;
	}


	void MACAdapter::getMotorOverrides(std::vector<float>& outOverrides) const
	{
		std::lock_guard<std::mutex> lock(mOverrideMutex);
		outOverrides = mMotorOverrides;
	}


	void MACAdapter::onCompute(const FlexDevice& device)
	{
		// Extract rope lengths
		device.getRopeLengths(mRopeLengths);

		// Get current motor override values
		std::vector<float> overrides;
		getMotorOverrides(overrides);

		// Apply overrides
		for (auto i = 0; i < overrides.size(); i++)
			mRopeLengths[i] += overrides[i] + mOverrideMinimum;
	}
}