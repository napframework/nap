// Local Includes
#include "maccontroller.h"

// External Includes
#include <soem/ethercat.h>

// nap::maccontroller run time class definition 
RTTI_BEGIN_CLASS(nap::MACController)
	// Put additional properties here
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
		reinterpret_cast<ec_slavet*>(slave)->PO2SOconfig = &MAC_SETUP;

		// Reset position if requested
		uint32_t new_pos = 0;
		sdoWrite(index, 0x2012, 0x04, FALSE, sizeof(new_pos), &new_pos);

		// Force motor on zero.
		uint32_t control_word = 0;
		control_word |= 1UL << 6;
		control_word |= 0x0 << 8;
		sdoWrite(index, 0x2012, 0x24, FALSE, sizeof(control_word), &control_word);
	}


	void MACController::onSafeOperational(void* slave, int index)
	{
		ec_slavet* cslave = reinterpret_cast<ec_slavet*>(slave);
		MAC_400_OUTPUTS* inputs = (MAC_400_OUTPUTS*)cslave->inputs;
	}


	void MACController::onProcess()
	{
		ec_slavet* slave = reinterpret_cast<ec_slavet*>(getSlave(1));

		// Read info
		MAC_400_INPUTS* inputs = (MAC_400_INPUTS*)slave->inputs;
		inputs->mErrorStatus;

		// Write info
		MAC_400_OUTPUTS* mac_outputs = (MAC_400_OUTPUTS*)slave->outputs;
		mac_outputs->mOperatingMode = 2;
		mac_outputs->mRequestedPosition = 1000000;
		mac_outputs->mVelocity = 2700;
		mac_outputs->mAcceleration = 360;
		mac_outputs->mTorque = 341;
	}
}