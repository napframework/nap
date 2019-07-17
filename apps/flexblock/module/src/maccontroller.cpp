// Local Includes
#include "maccontroller.h"

// External Includes
#include <soem/ethercat.h>

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
		ec_slavet* cslave = reinterpret_cast<ec_slavet*>(slave);
		MAC_400_INPUTS* inputs = (MAC_400_INPUTS*)cslave->inputs;
		mMotorPositions[index-1].mInitPosition = inputs->mActualPosition;
	}


	void MACController::onProcess()
	{
		int slave_count = getSlaveCount();
		assert(mMotorPositions.size() == slave_count);
		for (int i = 1; i <= slave_count; i++)
		{
			// Get slave to address
			ec_slavet* slave = reinterpret_cast<ec_slavet*>(getSlave(i));

			// Get relative motor position
			int32 req_position = mMotorPositions[i-1].mTargetPosition - mMotorPositions[i-1].mInitPosition;

			// Write info
			MAC_400_OUTPUTS* mac_outputs = (MAC_400_OUTPUTS*)slave->outputs;
			mac_outputs->mOperatingMode = 2;
			mac_outputs->mRequestedPosition = req_position;
			mac_outputs->mVelocity = mVelocity;
			mac_outputs->mAcceleration = mAcceleration;
			mac_outputs->mTorque = mTorque;
		}
	}


	void MACController::onInit()
	{
		mMotorPositions.clear();
		for (int i = 0; i < getSlaveCount(); i++)
		{
			mMotorPositions.emplace_back(MACPosition(mRequestedPosition));
		}
	}
}