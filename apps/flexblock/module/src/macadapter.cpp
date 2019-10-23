#include "macadapter.h"
#include "flexdevice.h"

#include <nap/logger.h>

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
		// Don't perform any tasks when the controller isn't running
		if (!mController->isRunning())
			return;

		// Extract rope lengths
		device.getRopeLengths(mRopeLengths);

		// convert meters to motorsteps
		for (auto& length : mRopeLengths)
		{
			length *= mMotorStepsPerMeter;
			length -= mMotorStepsPerMeter;
		}

		// check for all slaves to be operational and without errors
		// if any slave isn't operational stop processing
		for (int i = 0; i < mController->getSlaveCount(); i++)
		{
			if (mController->getSlaveState(i) != EtherCATMaster::ESlaveState::Operational)
			{
				nap::Logger::error("Slave: %d not operational! Stopping MACController!", i);
				mController->stop();
				return;
			}
		}

		// Ensure out input data matches number of slaves
		// This also ensures the initial set of motor positions matches that of the controller
		mController->getPositionData(mMotorData);

		// All slaves are operational
		for (int i = 0; i < mController->getSlaveCount(); i++)
		{
			// Ensure current index is not higher than our array of motor mappings
			// TODO: Should this be an assert?
			if(i >= mMotorMapping.size())
				continue;

			// get the mapped motor index
			int mapped_idx = mMotorMapping[i];

			// Check if the mapped index doesn't exceed number of slaves
			if (mapped_idx >= mController->getSlaveCount())
				continue;

			// Update target position
			mMotorData[i].setTargetPosition(mRopeLengths[mapped_idx]);

			// Continue if we don't have to process digital pin
			if (!mSetDigitalPin)
				continue;

			double target_meters = static_cast<double>(mMotorData[i].mTargetPosition) / static_cast<double>(mMotorStepsPerMeter);
			double curren_meters = static_cast<double>(mController->getActualPosition(i)) / static_cast<double>(mMotorStepsPerMeter);

			// Check if the pin needs to be activated and do so
			bool activateDigitalPin = target_meters - curren_meters > 0.02;
			mMotorData[i].setDigitalPin(0, activateDigitalPin);
		}

		// Update position data
		mController->setPositionData(mMotorData);
	}
}