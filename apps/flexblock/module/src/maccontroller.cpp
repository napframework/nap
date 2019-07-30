// Local Includes
#include "maccontroller.h"

// External Includes
#include <soem/ethercat.h>
#include <nap/logger.h>

// nap::maccontroller run time class definition 
RTTI_BEGIN_CLASS(nap::MACController)
	RTTI_PROPERTY("ResetPosition",			&nap::MACController::mResetPosition,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("ResetPositionValue",		&nap::MACController::mResetPositionValue,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Position",				&nap::MACController::mRequestedPosition,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Velocity",				&nap::MACController::mVelocity,				nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Acceleration",			&nap::MACController::mAcceleration,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Torque",					&nap::MACController::mTorque,				nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////
// Static callbacks
//////////////////////////////////////////////////////////////////////////

static int MAC_SETUP(uint16 slave)
{
	return 1;
}


//////////////////////////////////////////////////////////////////////////
// MAC inputs / outputs
//////////////////////////////////////////////////////////////////////////

/**
 * Data sent to slave
 */
typedef struct PACKED
{
	uint32_t	mOperatingMode;
	int32_t		mRequestedPosition;
	uint32_t	mVelocity;
	uint32_t	mAcceleration;
	uint32_t	mTorque;
	uint32_t	mAnalogueInput;
} MAC_400_OUTPUTS;


/**
 * Data received from slave
 */
typedef struct PACKED
{
	uint32_t	mOperatingMode;
	int32_t		mActualPosition;
	uint32_t	mActualVelocity;
	uint32_t	mAnalogueInput;
	uint32_t	mErrorStatus;
	uint32_t	mActualTorque;
	uint32_t	mFollowError;
	uint32_t	mActualTemperature;
} MAC_400_INPUTS;


//////////////////////////////////////////////////////////////////////////
// MACController
//////////////////////////////////////////////////////////////////////////

namespace nap
{
	MACController::~MACController()
	{
		mMotorParameters.clear();
	}


	void MACController::onPreOperational(void* slave, int index)
	{
		if (mResetPosition)
		{
			reinterpret_cast<ec_slavet*>(slave)->PO2SOconfig = &MAC_SETUP;

			// Reset position if requested
			sdoWrite(index, 0x2012, 0x04, FALSE, sizeof(mResetPositionValue), &mResetPositionValue);

			// Force motor on zero.
			uint32_t control_word = 0;
			control_word |= 1UL << 6;
			control_word |= 0x0 << 8;
			sdoWrite(index, 0x2012, 0x24, FALSE, sizeof(control_word), &control_word);
		}
	}


	void MACController::onSafeOperational(void* slave, int index)
	{
		// Store initial motor position
		ec_slavet* cslave = reinterpret_cast<ec_slavet*>(slave);
		MAC_400_INPUTS* inputs = (MAC_400_INPUTS*)cslave->inputs;
		mMotorParameters[index - 1]->mInitPosition = inputs->mActualPosition;

		// Force passive mode in safe operational mode
		setMotorMode(index, EMotorMode::Passive);

		// Ensure the motor is in passive mode.
		// uint32 new_mode = 20;
		// int size = -1;
		// sdoRead(index, 0x2012, 0x02, false, &size, &new_mode);
	}


	void MACController::onProcess()
	{
		int slave_count = getSlaveCount();
		assert(mMotorParameters.size() == slave_count);
		
		for (int i = 1; i <= slave_count; i++)
		{
			// Get slave to address
			ec_slavet* slave = reinterpret_cast<ec_slavet*>(getSlave(i));

			// Get relative motor position
			int32 req_position = mMotorParameters[i-1]->mTargetPosition - mMotorParameters[i-1]->mInitPosition;

			// Write info
			MAC_400_OUTPUTS* mac_outputs = (MAC_400_OUTPUTS*)slave->outputs;
			mac_outputs->mOperatingMode = static_cast<uint32_t>(EMotorMode::Position);
			mac_outputs->mRequestedPosition = req_position;
			mac_outputs->mVelocity = mVelocity;
			mac_outputs->mAcceleration = mAcceleration;
			mac_outputs->mTorque = mTorque;
		}
	}


	void MACController::onStart()
	{
		mMotorParameters.clear();
		for (int i = 0; i < getSlaveCount(); i++)
		{
			mMotorParameters.emplace_back(std::make_unique<MacOutputs>(mRequestedPosition));
		}
	}


	void MACController::onStop()
	{
		// Request new state
		EtherCATMaster::ESlaveState state = requestState(EtherCATMaster::ESlaveState::SafeOperational);
		if (state != EtherCATMaster::ESlaveState::SafeOperational)
			nap::Logger::warn("%s: not all slaves reached safe operational state", mID.c_str());

		// Set motor to passive mode
		for (int i = 1; i <= getSlaveCount(); i++)
			setMotorMode(i, EMotorMode::Passive);
	}


	void MACController::setMotorMode(int index, EMotorMode mode)
	{
		// Ensure the motor is in passive mode.
		uint32 new_mode = static_cast<nap::uint32>(mode);
		sdoWrite(index, 0x2012, 0x02, FALSE, sizeof(new_mode), &new_mode);
	}
}